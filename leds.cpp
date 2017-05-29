#define LED_ATTR_OFF      0
#define LED_ATTR_ON       1
#define LED_ATTR_BLINK    2
#define LED_ATTR_BLINK2   3

class CLeds
{
private:
  bool blinked_, blinked2_;
  const char *ledmap_;
  const char comport_, compin_;
  uint8_t mapsize_;
  volatile uint8_t ledstate_[16];
  volatile uint8_t ledsave_[16];
  
  bool com_state_;
  
  CTimer tmr_refr_, tmr_blink_;
  
  void adjust_pins()
  {
    CIO::adjust_opin(comport_, compin_, true);
    
    for (unsigned char n = 0; n < mapsize_; n += 2)
    {
      CIO::adjust_opin(ledmap_[n], ledmap_[n + 1], true);
      CIO::write(ledmap_[n], ledmap_[n + 1], false);
    }
  }
  
  void refresh_leds()
  {
    com_state_ = !com_state_;
    
    CIO::write(comport_, compin_, com_state_);
    
    for (unsigned char n = 0; n < mapsize_; n += 2)
    {
      uint8_t idx = n + (com_state_ ? 1 : 0);
      uint8_t state = ledstate_[idx];
      
      bool on = state == LED_ATTR_ON || 
        (state == LED_ATTR_BLINK && !blinked_) ||
          (state == LED_ATTR_BLINK2 && !blinked2_);
      
      if (com_state_) on = !on;
      
      CIO::write(ledmap_[n], ledmap_[n + 1], on);
    }
  }
  
public:
  CLeds(char const comport, char const compin,
        const char *ledmap, unsigned int mapsize) :
    comport_(comport), compin_(compin), 
    ledmap_(ledmap),
    mapsize_(mapsize),
    tmr_refr_(__leds_refresh), tmr_blink_(__leds_blink),
    com_state_(false), blinked_(false), blinked2_(false)
  {
    reset();
    
    adjust_pins();
    
    tmr_refr_.start();
    tmr_blink_.start();
  }
  
  ~CLeds()
  {
  }
  
  inline void tick()
  {
    tmr_refr_.tick();
    tmr_blink_.tick();
    
    if (tmr_refr_.elapsed())
      refresh_leds();
    
    if (tmr_blink_.elapsed())
    {
      if (blinked_) blinked2_ = !blinked2_;
      blinked_ = !blinked_;
    }
  }
  
  void set(uint8_t idx, uint8_t attr)
  {
    if (idx >= mapsize_) return;
    
    ledstate_[idx] = attr;
  }

  uint8_t get(uint8_t idx)
  {
    if (idx >= mapsize_) return 0;

    return ledstate_[idx];
  }
  
  void reset()
  {
    memset((uint8_t*)&ledstate_, 0, sizeof(ledstate_));
  }
  
  inline void push()
  {
    memcpy((uint8_t*)&ledsave_, (uint8_t*)&ledstate_, sizeof(ledstate_));
  }
  
  inline void pop()
  {
    memcpy((uint8_t*)&ledstate_, (uint8_t*)&ledsave_, sizeof(ledstate_));    
  }
};