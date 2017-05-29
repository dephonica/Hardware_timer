const long __timer_period = 65536 - 16000;
const uint8_t __timer_period_hi = (__timer_period >> 8) & 0xff;
const uint8_t __timer_period_lo = __timer_period & 0xff;

bool __timer_initialized = false;
volatile long long __timer_counter = 0;

#pragma vector = TIM1_OVR_UIF_vector
__interrupt void TIM1_OVR_UIF_handler(void)
{
  // Проверка, что же вызвало прерывание
  if (TIM1_SR1_UIF == 1)
  {
    TIM1_SR1_UIF = 0;             // Очистка флага прерывания по обновлению
    
    TIM1_CNTRH = __timer_period_hi;
    TIM1_CNTRL = __timer_period_lo;
    
    __timer_counter++;
  }  
}

class CTimer
{
private:
  bool enabled_, elapsed_;
  long period_;
  long long next_timer_tick_;
  
  static void init_timer()
  {
    // Настройка таймера 1
    TIM1_CR2 = 0;
    TIM1_SMCR = 0;
    TIM1_ETR = 0;
    TIM1_IER = MASK_TIM1_IER_UIE;
    TIM1_PSCRH = 0x0;
    TIM1_PSCRL = 0x0;

    TIM1_CNTRH = __timer_period_hi;
    TIM1_CNTRL = __timer_period_lo;
    
    TIM1_CR1 = (MASK_TIM1_CR1_URS | MASK_TIM1_CR1_CEN);
    
    __timer_initialized = true;
  }
  
public:
  CTimer(long period) : 
    period_(period), enabled_(false), elapsed_(false)
  {
    if (!__timer_initialized) init_timer();
  }
  
  ~CTimer()
  {
  }
  
  void start()
  {
    next_timer_tick_ = __timer_counter + period_;
    enabled_ = true;
    elapsed_ = false;
  }
  
  void stop()
  {
    enabled_ = false;
  }
    
  inline void tick()
  {
    if (!enabled_) return;
    
    if (__timer_counter >= next_timer_tick_)
    {
      next_timer_tick_ += period_;
      elapsed_ = true;
    }
  }

  inline void set(long period)
  {
    period_ = period;
  }
  
  inline bool elapsed()
  {
    if (elapsed_)
    {
      elapsed_ = false;
      return true;
    }
    
    return false;
  }
  
  inline bool enabled()
  {
    return enabled_;
  }

};
