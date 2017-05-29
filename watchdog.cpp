
#define WWDG_CR_RESET_VALUE ((unsigned char)0x7F);
#define WWDG_WR_RESET_VALUE ((unsigned char)0x7F);

class CWatchdog
{
private:
  
public:
  static void Init(unsigned char counter, unsigned char window)
  {
    WWDG_WR = WWDG_WR_RESET_VALUE;
    WWDG_CR = (unsigned char)(MASK_WWDG_CR_WDGA | MASK_WWDG_CR_T6 | counter);
    WWDG_WR = (unsigned char)((unsigned char)(~MASK_WWDG_CR_WDGA) & 
                              (unsigned char)(MASK_WWDG_CR_T6 | window));
  }
  
  inline static void set_counter(unsigned char counter)
  {
    unsigned char cc = WWDG_CR & (unsigned char)(~MASK_WWDG_CR_WDGA);
    
    if (cc < WWDG_WR)
        WWDG_CR = (unsigned char)(MASK_WWDG_CR_WDGA | 
                                  MASK_WWDG_CR_T6 | counter);
  }

  static unsigned char get_counter()
  {
    return WWDG_CR;
  }
};
