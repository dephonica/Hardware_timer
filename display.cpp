#define DISP_ATTR_NONE      0
#define DISP_ATTR_BLINK     1

#define BIN8(x) BIN___(0##x)
#define BIN___(x)                                        \
        (                                                \
        ((x / 01ul) % 010)*(2>>1) +                      \
        ((x / 010ul) % 010)*(2<<0) +                     \
        ((x / 0100ul) % 010)*(2<<1) +                    \
        ((x / 01000ul) % 010)*(2<<2) +                   \
        ((x / 010000ul) % 010)*(2<<3) +                  \
        ((x / 0100000ul) % 010)*(2<<4) +                 \
        ((x / 01000000ul) % 010)*(2<<5) +                \
        ((x / 010000000ul) % 010)*(2<<6)                 \
        )

struct __char_
{
  char ltr;
  unsigned char map;
};

const __char_ __disp_map[] = 
  {
    '0', BIN8(11111100),
    '1', BIN8(01100000),
    '2', BIN8(11011010),
    '3', BIN8(11110010),
    '4', BIN8(01100110),
    '5', BIN8(10110110),
    '6', BIN8(10111110),
    '7', BIN8(11100000),
    '8', BIN8(11111110),
    '9', BIN8(11110110),
    '-', BIN8(00000010),
    '_', BIN8(00010000),
    '`', BIN8(10000000),
    '.', BIN8(00000001)
  };

class CDisplay
{
private:
  char char_map_[128];

  char disp_pos_;  
  char value_[16];
  char attr_[16];

  char value_save_[16];
  char attr_save_[16];
  
  bool com_cathode_, blinked_;
  
  char port_, comport_;
  const uint8_t *pinmap_, *commap_;
  unsigned char comsize_;
  
  CTimer tmr_blink_, tmr_refr_;
  
  void generate_chars()
  {
    memset(&attr_, 0, sizeof(attr_));
    memset(&value_, 0, sizeof(value_));
    memset(&char_map_, com_cathode_ ? 0xff : 0, sizeof(char_map_));
    
    for (char n = 0; n < sizeof(__disp_map)/sizeof(__disp_map[0]); n++)
    {
      char sign = __disp_map[n].map;
      
      char result = 0;
      
      for (char m = 0; m < 8; m++, sign <<= 1)
        if (sign & 0x80)
          result |= (1 << pinmap_[m]);
      
      if (com_cathode_) result = ~result;
      
      char_map_[__disp_map[n].ltr] = result;
    }
  }

  void adjust_pins()
  {
    for (char m = 0; m < 8; m++)
      CIO::adjust_opin(port_, pinmap_[m], true);

    for (char m = 0; m < comsize_; m += 2)
      CIO::adjust_opin(commap_[m], commap_[m + 1], true);
  }
  
  void refresh()
  {
    unsigned char c_idx = disp_pos_ / 2;
    
    // Выключаем текущий разряд
    CIO::write(commap_[disp_pos_], commap_[disp_pos_ + 1], !com_cathode_);
    
    disp_pos_ += 2;
    if (disp_pos_ >= comsize_) disp_pos_ = 0;
    
    // Устанавливаем значение следующего разряда
    char byte = char_map_[value_[c_idx]];
    
    // Применяем атрибуты
    if (attr_[c_idx] == DISP_ATTR_BLINK)
      if (blinked_) byte = com_cathode_ ? 0xff : 0;
    
    CIO::write_byte(port_, byte);
    
    // Включаем отображение разряда
    CIO::write(commap_[disp_pos_], commap_[disp_pos_ + 1], com_cathode_);
  }
  
public:
  // port - буква порта, к которому подключен дисплей
  // pinmap - массив соответствия сегмента индикатора номеру вывода порта
  CDisplay(char port, const uint8_t *pinmap,
           const uint8_t *commap, unsigned char comsize,
           bool com_cathode, int refresh_t, int blink_t) :
    port_(port), pinmap_(pinmap),
    commap_(commap),
    com_cathode_(com_cathode),
    comsize_(comsize),
    tmr_refr_(refresh_t), tmr_blink_(blink_t),
    disp_pos_(0),
    blinked_(false)
  {
    generate_chars();
    adjust_pins();
    
    tmr_refr_.start();
    tmr_blink_.start();
  }
  
  ~CDisplay()
  {
  }
  
  inline void tick()
  {
    tmr_refr_.tick();
    tmr_blink_.tick();
    
    if (tmr_refr_.elapsed()) 
      refresh();
    if (tmr_blink_.elapsed()) blinked_ = !blinked_;
  }
  
  void set(const char *str)
  {
    strncpy(value_, str, 15);
  }

  void set_at(char pos, char value)
  {
    value_[pos] = value;
  }
  
  void set(int value)
  {
    signed char ptr = (comsize_ / 2) - 1;
    while (ptr >= 0)
    {
      unsigned char c = value % 10;
      value_[ptr--] = '0' + c;
      value /= 10;
    }
  }
  
  void set_attr(unsigned char pos, unsigned char attr)
  {
    attr_[pos] = attr;
  }
  
  void push()
  {
    memcpy(&value_save_, &value_, sizeof(value_));
    memcpy(&attr_save_, &attr_, sizeof(attr_));
  }
  
  void pop()
  {
    memcpy(&value_, &value_save_, sizeof(value_));
    memcpy(&attr_, &attr_save_, sizeof(attr_));
  }
};