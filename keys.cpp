// Период опроса клавиш, мсек.
#define KEYS_REFRESH_PERIOD     30
#define KEYS_PRESS_CHECKS       3

#define KEYS_MSG_UP             (1 << 6)
#define KEYS_MSG_DOWN           (2 << 6)
#define KEYS_MSG_PRESS          (3 << 6)

#define KEYS_MSG_QUEUE_SIZE     32

#define K_MSG(x)                (x & 0xc0)
#define K_KEY(x)                (x & 0x3f)

class CKeys
{
private:
  CTimer tmr_refr_, tmr_repeat_, tmr_repeat_key_;
  
  bool astate_;
  const uint8_t *keysdef_, defcount_;
  
  uint8_t keystate_[16], last_key_, keys_pressed_;
  uint8_t m_queue_[KEYS_MSG_QUEUE_SIZE], m_top_, m_bottom_;
  
  void adjust_pins()
  {
    for (uint8_t n = 0; n < defcount_; n += 2)
      CIO::adjust_ipin(keysdef_[n], keysdef_[n + 1], true);
  }

  void key_message(uint8_t key, uint8_t msg)
  {
    m_queue_[m_top_] = key | msg;
    
    m_top_++;
    if (m_top_ >= KEYS_MSG_QUEUE_SIZE) m_top_ = 0;
    
    if (m_top_ == m_bottom_) 
    {
      m_bottom_++;
      if (m_bottom_ >= KEYS_MSG_QUEUE_SIZE) m_bottom_ = 0;
    }
    
    if (msg == KEYS_MSG_DOWN) 
    {
      last_key_ = key;
      tmr_repeat_.start();
      tmr_repeat_key_.stop();
    } 
    else if (msg == KEYS_MSG_UP && key == last_key_)
    {
      tmr_repeat_key_.stop();
      tmr_repeat_.stop();
    }
    
    if (keys_pressed_ > 1) 
    {
      tmr_repeat_.stop();
      tmr_repeat_key_.stop();
    }
  }

  void refresh_keys()
  {
    uint8_t idx = 0;
    for (uint8_t n = 0; n < defcount_; n += 2, idx++)
    {
      bool in = CIO::read(keysdef_[n], keysdef_[n + 1]) == astate_;
      
      uint8_t state = keystate_[idx] & 0x7f;
      
      if (keystate_[idx] & 0x80)
      {
        // Клавиша в нажатом состоянии
        if (!in) state--;
        else state = KEYS_PRESS_CHECKS;
        
        if (state == 0) 
        {
          key_message(idx, KEYS_MSG_UP);
          keystate_[idx] = 0; // Клавиша отпущена
          keys_pressed_--;
        }
        else
          keystate_[idx] = state | 0x80;
      } else
      {
        // Клавиша в отпущеном состоянии
        if (in) state++;
        else state = 0;
        
        if (state == KEYS_PRESS_CHECKS)
        {
          keys_pressed_++;          
          key_message(idx, KEYS_MSG_DOWN);
          key_message(idx, KEYS_MSG_PRESS);
          keystate_[idx] = KEYS_PRESS_CHECKS | 0x80;  // Клавиша нажата
        }
        else
          keystate_[idx] = state;
      }
    }
  }
  
public:
  CKeys(const uint8_t *keysdef, uint8_t defcount,
        bool active_state) :
    keysdef_(keysdef), defcount_(defcount),
    tmr_refr_(KEYS_REFRESH_PERIOD),
    tmr_repeat_(__keys_repeat_wait),
    tmr_repeat_key_(__keys_repeat_time),
    astate_(active_state), keys_pressed_(0)
  {
    flush();
    
    memset(keystate_, 0, sizeof(keystate_));
    
    adjust_pins();
    
    tmr_refr_.start();
  }
  
  inline void tick()
  {
    tmr_refr_.tick();
    tmr_repeat_.tick();
    tmr_repeat_key_.tick();    
    
    if (tmr_refr_.elapsed()) refresh_keys();
    if (tmr_repeat_.elapsed()) tmr_repeat_key_.start();
    if (tmr_repeat_key_.elapsed())
      key_message(last_key_, KEYS_MSG_PRESS);
  }
  
  inline bool is_key(uint8_t idx)
  {
    if (idx >= defcount_ / 2) return false;
    return (keystate_[idx] & 0x80) != 0;
  }
  
  inline uint8_t get_msg()
  {
    if (m_top_ == m_bottom_) return 0;
    
    uint8_t r = m_queue_[m_bottom_];
    
    m_bottom_++;
    if (m_bottom_ >= KEYS_MSG_QUEUE_SIZE) m_bottom_ = 0;
    
    return r;
  }
  
  inline void flush()
  {
    m_top_ = m_bottom_ = 0;
    tmr_repeat_.stop();
    tmr_repeat_key_.stop();
  }
};
