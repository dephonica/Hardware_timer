
////////////////////////////////////////////////////////////////////////////
// Класс, реализующий работу с портами цифрового ввода-вывода
////////////////////////////////////////////////////////////////////////////
#define CREATE_MASK(trig, pin, mask1, mask2) \
  if (trig) { mask1 = 0xff; mask2 = 1 << pin; } \
    else { mask1 = ~(1 << pin); mask2 = 0x00; }

#define SETUP_PORT(ltr, portd, portc1, portc2) \
  if (port == ltr) { portd &= ddr_and_mask; portd |= ddr_or_mask; \
    portc1 &= cr1_and_mask; portc1 |= cr1_or_mask; \
      portc2 &= cr2_and_mask; portc2 |= cr2_or_mask; }

#define SWITCH_PIN(ltr, portord) \
    if (port == ltr) \
      { if (on) portord |= 1 << pin; else portord &= ~(1 << pin); } else


class CIO
{
private:
  static void adjust_pin(char port, uint8_t pin, bool output, bool cr1,
                         bool cr2)
  {
    port = CUtil::toupper(port);
    
    // Вычисляем AND и OR маски для регистра направления порта
    uint8_t ddr_and_mask = 0, ddr_or_mask = 0;
    CREATE_MASK(output, pin, ddr_and_mask, ddr_or_mask);
    
    // Вычисляем AND и OR маски для регистра подтяжки порта
    uint8_t cr1_and_mask = 0, cr1_or_mask = 0;
    CREATE_MASK(cr1, pin, cr1_and_mask, cr1_or_mask);
    
    // Вычисляем AND и OR маски для регистра прерывания порта
    uint8_t cr2_and_mask = 0, cr2_or_mask = 0;
    CREATE_MASK(cr2, pin, cr2_and_mask, cr2_or_mask);
    
    SETUP_PORT('A', PA_DDR, PA_CR1, PA_CR2);
    SETUP_PORT('B', PB_DDR, PB_CR1, PB_CR2);
    SETUP_PORT('C', PC_DDR, PC_CR1, PC_CR2);
    SETUP_PORT('D', PD_DDR, PD_CR1, PD_CR2);
    SETUP_PORT('E', PE_DDR, PE_CR1, PE_CR2);
    SETUP_PORT('F', PF_DDR, PF_CR1, PF_CR2);
    SETUP_PORT('G', PG_DDR, PG_CR1, PG_CR2);
    SETUP_PORT('H', PH_DDR, PH_CR1, PH_CR2);
    SETUP_PORT('I', PI_DDR, PI_CR1, PI_CR2);
  }
  
public:
  static void adjust_ipin(char port, uint8_t pin, bool pullup = false, 
                          bool raiseint = false)
  {
    adjust_pin(port, pin, false, pullup, raiseint);
  }
  
  static void adjust_opin(char port, uint8_t pin, bool pushpull = false)
  {
    adjust_pin(port, pin, true, pushpull, false);
  }
  
  static void write(char port, uint8_t pin, bool on)
  {
    port = CUtil::toupper(port);
    
    SWITCH_PIN('A', PA_ODR)
    SWITCH_PIN('B', PB_ODR)
    SWITCH_PIN('C', PC_ODR)
    SWITCH_PIN('D', PD_ODR)
    SWITCH_PIN('E', PE_ODR)
    SWITCH_PIN('F', PF_ODR)
    SWITCH_PIN('G', PG_ODR)
    SWITCH_PIN('H', PH_ODR)
    SWITCH_PIN('I', PI_ODR)
    {}
  }

  static void write_byte(char port, unsigned char byte)
  {
    port = CUtil::toupper(port);

    if (port == 'A') PA_ODR = byte;
    else if (port == 'B') PB_ODR = byte;
    else if (port == 'C') PC_ODR = byte;
    else if (port == 'D') PD_ODR = byte;
    else if (port == 'E') PE_ODR = byte;
    else if (port == 'F') PF_ODR = byte;
    else if (port == 'G') PG_ODR = byte;
    else if (port == 'H') PH_ODR = byte;
    else if (port == 'I') PI_ODR = byte;
  }
  
  static bool readout(char port, uint8_t pin)
  {
    port = CUtil::toupper(port);
    
    uint8_t r = 0;
    
    if (port == 'A') r = (PA_ODR >> pin) & 0x01;
    else if (port == 'B') r = (PB_ODR >> pin) & 0x01;
    else if (port == 'C') r = (PC_ODR >> pin) & 0x01;
    else if (port == 'D') r = (PD_ODR >> pin) & 0x01;
    else if (port == 'E') r = (PE_ODR >> pin) & 0x01;
    else if (port == 'F') r = (PF_ODR >> pin) & 0x01;
    else if (port == 'G') r = (PG_ODR >> pin) & 0x01;
    else if (port == 'H') r = (PH_ODR >> pin) & 0x01;
    else if (port == 'I') r = (PI_ODR >> pin) & 0x01;    
    
    return r != 0 ? true : false;
  }
  
  static bool read(char port, uint8_t pin)
  {
    port = CUtil::toupper(port);
    
    uint8_t r = 0;
    
    if (port == 'A') r = (PA_IDR >> pin) & 0x01;
    else if (port == 'B') r = (PB_IDR >> pin) & 0x01;
    else if (port == 'C') r = (PC_IDR >> pin) & 0x01;
    else if (port == 'D') r = (PD_IDR >> pin) & 0x01;
    else if (port == 'E') r = (PE_IDR >> pin) & 0x01;
    else if (port == 'F') r = (PF_IDR >> pin) & 0x01;
    else if (port == 'G') r = (PG_IDR >> pin) & 0x01;
    else if (port == 'H') r = (PH_IDR >> pin) & 0x01;
    else if (port == 'I') r = (PI_IDR >> pin) & 0x01;    
    
    return r != 0 ? true : false;
  }
};
