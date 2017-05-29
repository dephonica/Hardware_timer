
class CRelay
{
private:
  const bool on_state_;
  const uint8_t *rel_map_, rel_size_;
  
  void adjust_pins()
  {
    for (uint8_t n = 0; n < rel_size_; n += 2)
      CIO::adjust_opin(rel_map_[n], rel_map_[n + 1], true);
  }
  
public:
  CRelay(const uint8_t *rel_map, uint8_t rel_size, bool on_state) :
    rel_map_(rel_map), rel_size_(rel_size), on_state_(on_state)
  {
    adjust_pins();
  }
  
  void set(uint8_t idx, bool on)
  {
    idx *= 2;
    if (idx >= rel_size_) return;
    
    CIO::write(rel_map_[idx], rel_map_[idx + 1], on ? on_state_ : !on_state_);
  }
};
