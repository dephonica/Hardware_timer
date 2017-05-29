#define STATE_WORKING               1
#define STATE_CONFIGURING           2
#define STATE_CONFIGURING_ADVANCED  3

__eeprom __no_init unsigned long _eep_timers[5];
__eeprom __no_init unsigned long _eep_settings[2];
__eeprom __no_init unsigned long _eep_states[20][2];

#define EEP_KEY               0
#define EEP_STATE             1

unsigned char _eep_pointer = 0;
unsigned long _eep_key = 0;

// ����������� �������
#define STATE_1               1
#define STATE_2               2
#define STATE_3               3
#define STATE_4               4

// ��������� ������������ � �������� ������ ���������
struct state_mach
{
  uint8_t cur_state;        // ������� ���������, ��� �������� ����������� ������
  uint8_t msg_cond;         // ��� ��������� � ������� ��� �������� � ��������� ���������
  uint8_t in_cond;          // ����� ������� � �������� ������ ��������� ���������
  uint8_t next_state;       // ��������� ���������
  uint8_t relay1, relay2;   // ������ ����. ������������� ������� � ������� __rel_map
  uint8_t time1, time2;     // ������� �������� � ����������� � ������� __config
  uint8_t led_override;     // ����� ���������� �. ������������� ������� �������� CLeds
  uint8_t event;            // ��� �������. ��� ������� 1-3 �������� ���� ����� 1,
                            // ��� ������� 4-6 �������� ���� ����� 2
};

// ������� ��������� ����������� ������ ���������
const state_mach mach[] =
{
  STATE_1, IN_MSG_DOWN, 0, STATE_1, __rel1_o, __rel_v1, 0, 3, 2, 1,
  STATE_1, IN_MSG_UP, 0,   STATE_1, __rel2_z, __rel_v1, 0, 0, 2, 2,

  STATE_1, IN_MSG_DOWN, 1, STATE_1, __rel1_o, __rel_v2, 1, 3, 0, 1,
  STATE_1, IN_MSG_UP, 1,   STATE_1, __rel2_z, __rel_v2, 1, 0, 0, 2,

  STATE_1, IN_MSG_DOWN, 2, STATE_1, __rel1_o, __rel_v3, 2, 3, 1, 1,
  STATE_1, IN_MSG_UP, 2,   STATE_1, __rel2_z, __rel_v3, 2, 0, 1, 2,
};

uint8_t target_state_ = 0;

// ������ ������� ���������
const uint8_t mach_size = sizeof(mach) / sizeof(mach[0]);

// ��������� ������������ ������ ������ EEPROM �������� ����� ��������
// ����������, ������������� ��� ��������.
struct config_desc
{
  uint8_t timer_index;
  uint8_t led_index;
};

const config_desc __config[] = {0,3, 1,4, 2,5, 3,0, 4,0xff};

/////////////////////////////////////////////////////////////////////////////

class CBody
{
private:
  CRelay &relay_;
  CDisplay &disp_;
  CLeds &leds_;
  CKeys &keys_;
  CInputs &inp_;
  
  uint8_t state_;               // ������� ��������� - ��������� ��� ������
  uint8_t cfg_pointer_;         // ����� �������, ������� ����� �������� ���
                                // ��������� ������� ����������������

  uint8_t adv_cfg_pointer_;     // ����� ���������, ������� ����� ���������� ���
                                // ��������� ������� ���������� ����������������
                        
  uint8_t w_state_;             // ������� ��������� - ������������� ������ ��
                                // cur_state � ������� ��������� ������ ���������
  
  const state_mach *active_state_;  // ������ �� �������� ������ ������� ���������
  
  bool cfg_changed_;            // ���� ��������� ��������� ������� - ����� �������� EEPROM
  long cfg_value_;              // �������� ������� ��� �������� ������ ���������

  CTimer tmr_key_config_;       // ������ ��� ��������� ������ ���������
  CTimer tmr_key_adv_config_;   // ������ ��� ��������� ������ ��������� ��������
  CTimer tmr_config_exit_;      // ������ ��� ������ �� ������ ���������
  
  CTimer tmr_t1_, tmr_t2_;      // �������� �������, ����������� ����

/////////////////////////////////////////////////////////////////////////////
    
  inline void reset_states()
  {
    active_state_ = NULL;

    // ��������� �����������
    relay_.set(__rel_v1, false);
    relay_.set(__rel_v2, false);
    relay_.set(__rel_v3, false);
    
    // N ������ ��������� ������
    // ������������� ������ 1 �� ������ ����� ���������� ������
    long time = _eep_timers[__config[4].timer_index];
    
    tmr_t1_.set(time * 1000L);
    tmr_t1_.start();
    
    // �������� ���� ����������
    relay_.set(__rel2_z, true);
    
    // �������� ��������� �� �������
    disp_.set("---");
    disp_.set_attr(0, DISP_ATTR_BLINK);
    disp_.set_attr(1, DISP_ATTR_BLINK);
    disp_.set_attr(2, DISP_ATTR_BLINK);
  }
  
/////////////////////////////////////////////////////////////////////////////
// ����� ���������������� - ����������� ��������, � ��������� � ����������
//
  
  inline void init_configure()
  {
    leds_.push();
    disp_.push();
    
    cfg_changed_ = false;
    cfg_value_ = _eep_timers[__config[cfg_pointer_].timer_index];
    
    // ��������� ��� ����������
    leds_.reset();
    
    // �������� ��������������� ��������� �� �������
    uint8_t led = __config[cfg_pointer_].led_index;
    
    if (led != 0xff)
      leds_.set(led, LED_ATTR_BLINK);
    else
    {
      leds_.set(0, LED_ATTR_BLINK);
      leds_.set(1, LED_ATTR_BLINK);
      leds_.set(2, LED_ATTR_BLINK);
      leds_.set(3, LED_ATTR_BLINK);
      leds_.set(4, LED_ATTR_BLINK);
      leds_.set(5, LED_ATTR_BLINK);
    }

    if (led == 0) 
    {
      leds_.set(1, LED_ATTR_BLINK);
      leds_.set(2, LED_ATTR_BLINK);
    }
    
    // ���������� ������� �������� ��������
    disp_.set(cfg_value_);
   
    // ��������� ������ ������ �� ������ ����������������
    tmr_config_exit_.start();
    
    // ������� ������� ��������� ����������
    keys_.flush();
  }
  
  inline void do_configure()
  {
    tmr_config_exit_.tick();
    if (tmr_config_exit_.elapsed())
    {
      // ����� �� ������ ����������������
      // ������������� ������
      tmr_config_exit_.stop();

      // ���� �������� ������� ��������, �������� ���
      if (cfg_changed_) 
      {
        CEEPROM::unlock();
        _eep_timers[__config[cfg_pointer_].timer_index] = cfg_value_;
        CEEPROM::lock();
      }
      
      cfg_pointer_++;
      if (cfg_pointer_ >= sizeof(__config) / sizeof(__config[0]))
        cfg_pointer_ = 0;

      leds_.pop();
      disp_.pop();
      
      state_ = STATE_WORKING;
      
      // ���������� ��� ��������� �� ������� ��������
      inp_.flush();
      
      return;
    }
    
    uint8_t msg = keys_.get_msg();
    
    if (K_MSG(msg) == KEYS_MSG_PRESS)
    {
      // �������� ������ ������ ������� ������
      tmr_config_exit_.start();
     
      cfg_changed_ = true;
      
      if (K_KEY(msg) == __keys_keyup)
        cfg_value_++;
      else
        cfg_value_--;
      
      if (cfg_value_ < 0) cfg_value_ = 999;
      if (cfg_value_ > 999) cfg_value_ = 0;
      
      disp_.set(cfg_value_);
    }
  }

///////////////////////////////////////////////////////////////////////////////////////
// ����� ���������� ���������������� - ����������� ��������, �� ��������� � ����������
//
  
  inline void init_configure_advanced()
  {
    leds_.push();
    disp_.push();
    
    cfg_changed_ = false;
    cfg_value_ = _eep_settings[adv_cfg_pointer_];
    
    // ��������� ��� ����������
    leds_.reset();
        
    // ���������� ������� �������� ���������
    disp_.set_at(0, '0' + (cfg_value_ / 10));
    disp_.set_at(1, adv_cfg_pointer_ == 0 ? '`' : '_');
    disp_.set_at(2, '0' + (cfg_value_ % 10));
   
    // ��������� ������ ������ �� ������ ����������������
    tmr_config_exit_.start();
    
    // ������� ������� ��������� ����������
    keys_.flush();
  }
  
  inline void do_configure_advanced()
  {
    tmr_config_exit_.tick();
    if (tmr_config_exit_.elapsed())
    {
      // ����� �� ������ ����������������
      // ������������� ������
      tmr_config_exit_.stop();

      // ���� �������� ��������� ��������, �������� ���
      if (cfg_changed_) 
      {
        CEEPROM::unlock();
        _eep_settings[adv_cfg_pointer_] = cfg_value_;
        CEEPROM::lock();
      }
      
      adv_cfg_pointer_++;
      if (adv_cfg_pointer_ >= sizeof(_eep_settings) / sizeof(_eep_settings[0]))
        adv_cfg_pointer_ = 0;

      leds_.pop();
      disp_.pop();

      // ������������������ �������� ���������� �������� �� EEPROM
      __in_count_active = _eep_settings[0] * 100;
      __in_count_inactive = _eep_settings[1] * 100;
      
      state_ = STATE_WORKING;
      
      // ���������� ��� ��������� �� ������� ��������
      inp_.flush();
      
      return;
    }
    
    uint8_t msg = keys_.get_msg();
    
    if (K_MSG(msg) == KEYS_MSG_PRESS)
    {
      // �������� ������ ������ ������� ������
      tmr_config_exit_.start();
     
      cfg_changed_ = true;
      
      if (K_KEY(msg) == __keys_keyup)
        cfg_value_++;
      else
        cfg_value_--;
      
      if (cfg_value_ < 0) cfg_value_ = 99;
      if (cfg_value_ > 99) cfg_value_ = 0;
      
      disp_.set_at(0, '0' + (cfg_value_ / 10));
      disp_.set_at(1, adv_cfg_pointer_ == 0 ? '`' : '_');
      disp_.set_at(2, '0' + (cfg_value_ % 10));
    }
  }

  // ��������� ������� ��������� ������ ����������������  
  inline bool check_config_entry()
  {
    // ��������� ������� ��������� ��������� ��������
    if ((keys_.is_key(__keys_keyup) && !keys_.is_key(__keys_keydown)) &&
        w_state_ == STATE_1)
    {
      if (tmr_key_adv_config_.enabled())
      {
        if (tmr_key_adv_config_.elapsed())
        {
          tmr_key_adv_config_.stop();

          state_ = STATE_CONFIGURING_ADVANCED;
          init_configure_advanced();
          return true;
        }
      } else tmr_key_adv_config_.start();
    } else tmr_key_adv_config_.stop();
    
    // ��������� ������� ��������� ���������������� ��������
    if ((keys_.is_key(__keys_keyup) && keys_.is_key(__keys_keydown)) &&
        w_state_ == STATE_1)
    {
      if (tmr_key_config_.enabled())
      {
        if (tmr_key_config_.elapsed())
        {
          tmr_key_config_.stop();

          state_ = STATE_CONFIGURING;
          init_configure();
          return true;
        }
      } else tmr_key_config_.start();
    } else tmr_key_config_.stop();
    
    return false;
  }

//////////////////////////////////////////////////////////////////////////////
// �������� ������� ���� - �������� ������, ��������� � ����������� ��������
//

  inline void handle_event(const state_mach &e)
  {
    // ��������� ������� ��������� ��� ������������� �
    // ������������ �������� ��������
    active_state_ = &e;
    
    //save_state_to_eep(e);
    
    if (e.event == 1)
    {
      // ��������� ������� � ���������� 1-3
      
      // ����������� �������
      long time1 = _eep_timers[__config[e.time1].timer_index];
      long time2 = _eep_timers[__config[e.time2].timer_index];
      
      tmr_t1_.set(time1 * 1000);
      tmr_t2_.set(time2 * 1000);
      
      // �������� ���������� �� �������
      leds_.set(__config[e.time1].led_index, LED_ATTR_BLINK);
      leds_.set(e.led_override, LED_ATTR_BLINK);
      
      // �������� ���� 1(O)
      relay_.set(e.relay1, true);
      
      // ��������� �������
      tmr_t1_.start();
      tmr_t2_.start();
    } else if (e.event == 2)
    {
      // ��������� ������� � ���������� 4-6
      
      // ����������� ������
      long time1 = _eep_timers[__config[e.time1].timer_index];
      
      tmr_t1_.set(time1 * 1000);
      
      // ������������� ����������
      leds_.set(__config[e.time1].led_index, LED_ATTR_BLINK);
      leds_.set(e.led_override, LED_ATTR_OFF);
      
      // ������������� ����
      relay_.set(e.relay1, true);
      relay_.set(e.relay2, false);

      // ��������� ������
      tmr_t1_.start();
    }
  }

  void init_target_state()
  {
    inp_.flush();
        
    if (!inp_.get(0)) 
    {
      target_state_ = STATE_2;
      if (!inp_.get(1)) 
      {
        target_state_ = STATE_3;
        if (!inp_.get(2)) target_state_ = STATE_4;
      }
    }
  }
  
  // ��������� ������������ ������� 1(O) ��� 2(�)
  void handle_event_timer1()
  {
    // ������������� ������
    tmr_t1_.stop();
    
    if (active_state_ == NULL)
    {
      // ��������� ���� ���������� ������
      relay_.set(__rel2_z, false);
    
      // �������� ���������
      disp_.set_attr(0, DISP_ATTR_NONE);
      disp_.set_attr(1, DISP_ATTR_NONE);
      disp_.set_attr(2, DISP_ATTR_NONE);

      //init_target_state();
      
      return;
    }
    
    // ��������� ���� 1(O) ��� 2(�)
    relay_.set(active_state_->relay1, false);

    if (active_state_->event == 1) 
      // �������� ��������� �� ���������� ��������
      leds_.set(__config[active_state_->time1].led_index, LED_ATTR_ON);
    else
      // ��������� ���������
      leds_.set(__config[active_state_->time1].led_index, LED_ATTR_OFF);
  }

  // ��������� ������������ ������� �
  void handle_event_timer2()
  {
    // ��������!
    // ���������� ����������� ������ � ���������� 1-3
    // � ���������� 4-6 ������ 2 �� ������������
    
    // ������������� ������
    tmr_t2_.stop();
        
    // �������� ���� �
    relay_.set(active_state_->relay2, true);
        
    // �������� ��������� �� ���������� ��������
    leds_.set(active_state_->led_override, LED_ATTR_ON);
  }

  inline void startup_sequence()
  {
    if (w_state_ < target_state_)
    {
      // ���� ��������� �����
      for (uint8_t n = 0; n < mach_size; n++)
        if (mach[n].cur_state == w_state_ &&
            mach[n].msg_cond == IN_MSG_DOWN)
        {
          // �������� ���������
          w_state_ = mach[n].next_state;
          
          if (w_state_ == target_state_)
            target_state_ = 0;
            
          // ������������ �������
          handle_event(mach[n]);
          
          break;
        }          
    }
  }
  
  inline void common_sequence()
  {
    uint8_t msg = inp_.get_msg();
    
    if (msg)
    {
      // �� �������� ��������� ����� �������
      
      for (uint8_t n = 0; n < mach_size; n++)
        if (w_state_ == mach[n].cur_state)
        {
          // ����� ������� ��������� � ������� ���������
          // ��������� ���������� �������
          
          if (I_MSG(msg) == mach[n].msg_cond &&
              I_IN(msg) == mach[n].in_cond)
          {
            // ������� ��������� - ����� ������� � ����� �������
            // ��������� � ��������� � �������
            
            // �������� ���������
            w_state_ = mach[n].next_state;
            
            // ������������ �������
            handle_event(mach[n]);
          }
        }
    }
  }
  
  inline void do_work()
  {
    // ��������� ������� �������� � ����� ����������������
    if (check_config_entry()) return;

    // ������������ ������� ������ ����� ����, ��� �������
    // �����������. ��� �������, ������������ � ������ ������
    // �������� ����������� � ������� � ��������������� �������
    // � ����� ���������� �����, ����� ��������� ��������
    if (!tmr_t1_.enabled() && !tmr_t2_.enabled())
    {
      // ��������� ����� ������ - ���������������� �����������
      // ���������
//      if (target_state_ != 0) startup_sequence();
//      else 
        // ������� ����� ������ - online ������� �� �����
        common_sequence();
    }
  }
    
public:
  CBody(CDisplay &disp, CLeds &leds, CKeys &keys, CInputs &inp,
        CRelay &relay) :
    disp_(disp), leds_(leds), keys_(keys), inp_(inp), relay_(relay),
    tmr_key_config_(__menu_press_time), tmr_key_adv_config_(__adv_menu_press_time), 
    tmr_config_exit_(__menu_wait_time),
    tmr_t1_(0), tmr_t2_(0),
    cfg_pointer_(0), adv_cfg_pointer_(0),
    active_state_(NULL)
  {
    // ���������� ��������� � ��������
    state_ = -1;
    w_state_ = -1;

    reset_states();

    state_ = STATE_WORKING;
    w_state_ = STATE_1;

    __in_count_active = _eep_settings[0] * 100;
    __in_count_inactive = _eep_settings[1] * 100;
  }
  
  inline void tick()
  {
    tmr_key_config_.tick();
    tmr_key_adv_config_.tick();

    tmr_t1_.tick();
    tmr_t2_.tick();
    
    if (tmr_t1_.elapsed()) handle_event_timer1();
    if (tmr_t2_.elapsed()) handle_event_timer2();
    
    if (state_ == STATE_WORKING) do_work();
    else if (state_ == STATE_CONFIGURING) do_configure();
    else if (state_ == STATE_CONFIGURING_ADVANCED) do_configure_advanced();
  }
};
