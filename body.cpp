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

// Определения событий
#define STATE_1               1
#define STATE_2               2
#define STATE_3               3
#define STATE_4               4

// Структура используемая в описании машины состояний
struct state_mach
{
  uint8_t cur_state;        // Текущее состояние, для которого описывается строка
  uint8_t msg_cond;         // Тип сообщения с датчика для перехода в следующее состояние
  uint8_t in_cond;          // Номер датчика с которого должно поступить сообщение
  uint8_t next_state;       // Следующее состояние
  uint8_t relay1, relay2;   // Номера реле. Соответствуют индексу в массиве __rel_map
  uint8_t time1, time2;     // Индексы задержек и светодиодов в массиве __config
  uint8_t led_override;     // Номер светодиода В. Соответствует индексу драйвера CLeds
  uint8_t event;            // Тип события. Для событий 1-3 значение поля равно 1,
                            // для событий 4-6 значение поля равно 2
};

// Таблица переходов описывающая машину состояний
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

// Размер таблицы состояний
const uint8_t mach_size = sizeof(mach) / sizeof(mach[0]);

// Настройка соответствия номера ячейки EEPROM хранящей время задержки
// светодиоду, отображающему эту задержку.
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
  
  uint8_t state_;               // Текущее состояние - настройка или работа
  uint8_t cfg_pointer_;         // Номер таймера, который будет настроен при
                                // следующем запуске конфигурирования

  uint8_t adv_cfg_pointer_;     // Номер настройки, которая будет отображена при
                                // следующем запуске системного конфигурирования
                        
  uint8_t w_state_;             // Рабочее состояние - соответствует одному из
                                // cur_state в таблице переходов машины состояний
  
  const state_mach *active_state_;  // Ссылка на активную строку таблицы переходов
  
  bool cfg_changed_;            // Флаг изменения настройки таймера - нужно обновить EEPROM
  long cfg_value_;              // Значение таймера при активном режиме настройки

  CTimer tmr_key_config_;       // Таймер для активации режима настройки
  CTimer tmr_key_adv_config_;   // Таймер для активации режима системных настроек
  CTimer tmr_config_exit_;      // Таймер для выхода из режима настройки
  
  CTimer tmr_t1_, tmr_t2_;      // Основные таймеры, управляющие реле

/////////////////////////////////////////////////////////////////////////////
    
  inline void reset_states()
  {
    active_state_ = NULL;

    // Выключаем вентиляторы
    relay_.set(__rel_v1, false);
    relay_.set(__rel_v2, false);
    relay_.set(__rel_v3, false);
    
    // N секунд закрываем жалюзи
    // Устанавливаем таймер 1 на полное время закрывания жалюзи
    long time = _eep_timers[__config[4].timer_index];
    
    tmr_t1_.set(time * 1000L);
    tmr_t1_.start();
    
    // Включаем реле закрывания
    relay_.set(__rel2_z, true);
    
    // Включаем светодиод на мигание
    disp_.set("---");
    disp_.set_attr(0, DISP_ATTR_BLINK);
    disp_.set_attr(1, DISP_ATTR_BLINK);
    disp_.set_attr(2, DISP_ATTR_BLINK);
  }
  
/////////////////////////////////////////////////////////////////////////////
// Режим конфигурирования - отображение задержки, её изменение и сохранение
//
  
  inline void init_configure()
  {
    leds_.push();
    disp_.push();
    
    cfg_changed_ = false;
    cfg_value_ = _eep_timers[__config[cfg_pointer_].timer_index];
    
    // Выключаем все светодиоды
    leds_.reset();
    
    // Включаем конфигурируемый светодиод на мигание
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
    
    // Отображаем текущее значение задержки
    disp_.set(cfg_value_);
   
    // Запускаем таймер выхода из режима конфигурирования
    tmr_config_exit_.start();
    
    // Очищаем очередь сообщений клавиатуры
    keys_.flush();
  }
  
  inline void do_configure()
  {
    tmr_config_exit_.tick();
    if (tmr_config_exit_.elapsed())
    {
      // Выход из режима конфигурирования
      // Останавливаем таймер
      tmr_config_exit_.stop();

      // Если значение времени изменено, сохраним его
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
      
      // Сбрасываем все сообщения из очереди датчиков
      inp_.flush();
      
      return;
    }
    
    uint8_t msg = keys_.get_msg();
    
    if (K_MSG(msg) == KEYS_MSG_PRESS)
    {
      // Начинаем заново отсчёт таймера выхода
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
// Режим системного конфигурирования - отображение настроек, их изменение и сохранение
//
  
  inline void init_configure_advanced()
  {
    leds_.push();
    disp_.push();
    
    cfg_changed_ = false;
    cfg_value_ = _eep_settings[adv_cfg_pointer_];
    
    // Выключаем все светодиоды
    leds_.reset();
        
    // Отображаем текущее значение параметра
    disp_.set_at(0, '0' + (cfg_value_ / 10));
    disp_.set_at(1, adv_cfg_pointer_ == 0 ? '`' : '_');
    disp_.set_at(2, '0' + (cfg_value_ % 10));
   
    // Запускаем таймер выхода из режима конфигурирования
    tmr_config_exit_.start();
    
    // Очищаем очередь сообщений клавиатуры
    keys_.flush();
  }
  
  inline void do_configure_advanced()
  {
    tmr_config_exit_.tick();
    if (tmr_config_exit_.elapsed())
    {
      // Выход из режима конфигурирования
      // Останавливаем таймер
      tmr_config_exit_.stop();

      // Если значение параметра изменено, сохраним его
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

      // Переинициализируем значения параметров датчиков из EEPROM
      __in_count_active = _eep_settings[0] * 100;
      __in_count_inactive = _eep_settings[1] * 100;
      
      state_ = STATE_WORKING;
      
      // Сбрасываем все сообщения из очереди датчиков
      inp_.flush();
      
      return;
    }
    
    uint8_t msg = keys_.get_msg();
    
    if (K_MSG(msg) == KEYS_MSG_PRESS)
    {
      // Начинаем заново отсчёт таймера выхода
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

  // Проверяем условия включения режима конфигурирования  
  inline bool check_config_entry()
  {
    // Проверяем условие активации системных настроек
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
    
    // Проверяем условие активации пользовательских настроек
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
// Основной рабочий цикл - проверка входов, отработка и отображение задержек
//

  inline void handle_event(const state_mach &e)
  {
    // Сохраняем текущее состояние для использования в
    // обработчиках таймеров задержки
    active_state_ = &e;
    
    //save_state_to_eep(e);
    
    if (e.event == 1)
    {
      // Обработка событий в состояниях 1-3
      
      // Настраиваем таймеры
      long time1 = _eep_timers[__config[e.time1].timer_index];
      long time2 = _eep_timers[__config[e.time2].timer_index];
      
      tmr_t1_.set(time1 * 1000);
      tmr_t2_.set(time2 * 1000);
      
      // Включаем светодиоды на мигание
      leds_.set(__config[e.time1].led_index, LED_ATTR_BLINK);
      leds_.set(e.led_override, LED_ATTR_BLINK);
      
      // Включаем реле 1(O)
      relay_.set(e.relay1, true);
      
      // Запускаем таймеры
      tmr_t1_.start();
      tmr_t2_.start();
    } else if (e.event == 2)
    {
      // Обработка событий в состояниях 4-6
      
      // Настраиваем таймер
      long time1 = _eep_timers[__config[e.time1].timer_index];
      
      tmr_t1_.set(time1 * 1000);
      
      // Устанавливаем светодиоды
      leds_.set(__config[e.time1].led_index, LED_ATTR_BLINK);
      leds_.set(e.led_override, LED_ATTR_OFF);
      
      // Устанавливаем реле
      relay_.set(e.relay1, true);
      relay_.set(e.relay2, false);

      // Запускаем таймер
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
  
  // Обработка срабатывания таймера 1(O) или 2(З)
  void handle_event_timer1()
  {
    // Останавливаем таймер
    tmr_t1_.stop();
    
    if (active_state_ == NULL)
    {
      // Выключаем реле закрывания жалюзи
      relay_.set(__rel2_z, false);
    
      // Вылючаем светодиод
      disp_.set_attr(0, DISP_ATTR_NONE);
      disp_.set_attr(1, DISP_ATTR_NONE);
      disp_.set_attr(2, DISP_ATTR_NONE);

      //init_target_state();
      
      return;
    }
    
    // Выключаем реле 1(O) или 2(З)
    relay_.set(active_state_->relay1, false);

    if (active_state_->event == 1) 
      // Включаем светодиод на постоянное свечение
      leds_.set(__config[active_state_->time1].led_index, LED_ATTR_ON);
    else
      // Выключаем светодиод
      leds_.set(__config[active_state_->time1].led_index, LED_ATTR_OFF);
  }

  // Обработка срабатывания таймера В
  void handle_event_timer2()
  {
    // Внимание!
    // Обработчик срабатывает только в состояниях 1-3
    // В состояниях 4-6 таймер 2 не активируется
    
    // Останавливаем таймер
    tmr_t2_.stop();
        
    // Включаем реле В
    relay_.set(active_state_->relay2, true);
        
    // Включаем светодиод на постоянное свечение
    leds_.set(active_state_->led_override, LED_ATTR_ON);
  }

  inline void startup_sequence()
  {
    if (w_state_ < target_state_)
    {
      // Ищем следующий режим
      for (uint8_t n = 0; n < mach_size; n++)
        if (mach[n].cur_state == w_state_ &&
            mach[n].msg_cond == IN_MSG_DOWN)
        {
          // Изменяем состояние
          w_state_ = mach[n].next_state;
          
          if (w_state_ == target_state_)
            target_state_ = 0;
            
          // Обрабатываем событие
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
      // На датчиках произошло новое событие
      
      for (uint8_t n = 0; n < mach_size; n++)
        if (w_state_ == mach[n].cur_state)
        {
          // Нашли текущее состояние в таблице состояний
          // Проверяем выполнение условий
          
          if (I_MSG(msg) == mach[n].msg_cond &&
              I_IN(msg) == mach[n].in_cond)
          {
            // Условия выполнены - фронт сигнала и номер датчика
            // совпадают с заданными в таблице
            
            // Изменяем состояние
            w_state_ = mach[n].next_state;
            
            // Обрабатываем событие
            handle_event(mach[n]);
          }
        }
    }
  }
  
  inline void do_work()
  {
    // Проверяем событие перехода в режим конфигурирования
    if (check_config_entry()) return;

    // Обрабатываем события только после того, как таймеры
    // остановлены. Все события, происходящие в момент работы
    // таймеров сохраняются в очереди в хронологическом порядке
    // и будут обработаны позже, после остановки таймеров
    if (!tmr_t1_.enabled() && !tmr_t2_.enabled())
    {
      // Стартовый режим работы - последовательное подключение
      // устройств
//      if (target_state_ != 0) startup_sequence();
//      else 
        // Штатный режим работы - online реакция на входы
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
    // Сбрасываем состояние в исходное
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
