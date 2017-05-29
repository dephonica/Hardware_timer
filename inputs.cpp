#define MAX_INPUTS      16

#define IN_MSG_UP             (1 << 6)
#define IN_MSG_DOWN           (2 << 6)

#define IN_MSG_QUEUE_SIZE     32

#define I_MSG(x)                (x & 0xc0)
#define I_IN(x)                 (x & 0x3f)

class CInputs
{
private:
  uint16_t in_count_[MAX_INPUTS];
  bool in_state_[MAX_INPUTS];
  
  uint8_t m_queue_[IN_MSG_QUEUE_SIZE], m_top_, m_bottom_;
  
  const uint8_t *inmap_, insize_;
  
  CTimer tmr_read_, tmr_ref_;
  
  void adjust_pins()
  {
    for (uint8_t n = 0; n < insize_; n += 2)
      CIO::adjust_ipin(inmap_[n], inmap_[n + 1], true);
  }
  
  void in_message(uint8_t inp, uint8_t msg)
  {
    m_queue_[m_top_] = inp | msg;
    
    m_top_++;
    if (m_top_ >= IN_MSG_QUEUE_SIZE) m_top_ = 0;
    
    if (m_top_ == m_bottom_) 
    {
      m_bottom_++;
      if (m_bottom_ >= IN_MSG_QUEUE_SIZE) m_bottom_ = 0;
    }
  }
  
  inline void read()
  {
    int idx = 0;
    for (uint8_t n = 0; n < insize_; n += 2, idx++)
    {
      bool in = CIO::read(inmap_[n], inmap_[n + 1]);
      
      if (in && in_count_[idx] < 65535) 
        in_count_[idx]++;
    }
  }
  
  inline void map()
  {
    int idx = 0;
    for (uint8_t n = 0; n < insize_; n += 2, idx++)
    {
      bool state = true;
      if (in_count_[idx] < __in_count_active) state = false;
      
      if (state != in_state_[idx])
      {
        if (state)
          in_message(idx, IN_MSG_UP);
        else
          in_message(idx, IN_MSG_DOWN);
        
        in_state_[idx] = state;
      }
      
      in_count_[idx] = 0;
    }
  }
  
public:
  CInputs(const uint8_t *inmap, uint8_t insize) :
    inmap_(inmap), insize_(insize),
    tmr_read_(__in_read_period), tmr_ref_(__in_ref_period)
  {
    flush();
    
    memset(in_count_, 0, sizeof(in_count_));
    memset(in_state_, 1, sizeof(in_state_));
    
    adjust_pins();
    
    tmr_read_.start();
    tmr_ref_.start();
  }
  
  inline void tick()
  {
    tmr_read_.tick();
    tmr_ref_.tick();
    
    if (tmr_read_.elapsed()) read();
    if (tmr_ref_.elapsed()) map();
  }
  
  inline uint8_t get_msg()
  {
    if (m_top_ == m_bottom_) return 0;
    
    uint8_t r = m_queue_[m_bottom_];
    
    m_bottom_++;
    if (m_bottom_ >= IN_MSG_QUEUE_SIZE) m_bottom_ = 0;
    
    return r;
  }
  
  inline void flush()
  {
    m_top_ = m_bottom_ = 0;
  }
  
  inline bool get(uint8_t idx)
  {
    if (idx >= insize_) return false;
    
    return in_state_[idx];
  }
};