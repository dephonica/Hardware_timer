
typedef enum {
    FLASH_FLAG_HVOFF     = (unsigned char)0x40,     /*!< End of high voltage flag */
    FLASH_FLAG_DUL       = (unsigned char)0x08,     /*!< Data EEPROM unlocked flag */
    FLASH_FLAG_EOP       = (unsigned char)0x04,     /*!< End of programming (write or erase operation) flag */
    FLASH_FLAG_PUL       = (unsigned char)0x02,     /*!< Flash Program memory unlocked flag */
    FLASH_FLAG_WR_PG_DIS = (unsigned char)0x01      /*!< Write attempted to protected page flag */
} FLASH_Flag_TypeDef;

#define OPERATION_TIMEOUT  ((unsigned short)0x1000)

extern "C"
{
  int __eeprom_wait_for_last_operation()
  {
      bool flagstatus = true;
      unsigned short timeout = OPERATION_TIMEOUT;
    
      /* Wait until operation completion or write protected page occured */
      while ((flagstatus == 0x00) && (timeout != 0x00))
      {
          flagstatus = (uint8_t)(FLASH_IAPSR & (MASK_FLASH_IAPSR_HVOFF |
                                            MASK_FLASH_IAPSR_WR_PG_DIS));
          timeout--;
      }

      if (timeout == 0x00)
          flagstatus = false;

      return flagstatus;
  }

  void __eeprom_program_byte(unsigned char __near * dst, unsigned char v)
  {
      *((__near uint8_t*) dst) = v;
  }

  void __eeprom_program_long(unsigned char __near * dst, unsigned long v)
  {
      FLASH_CR2 |= MASK_FLASH_CR2_WPRG;
      FLASH_NCR2 &= (uint8_t)(~MASK_FLASH_NCR2_NWPRG);

      *((__near uint8_t*)dst)       = *((uint8_t*)(&v));
      *(((__near uint8_t*)dst) + 1) = *((uint8_t*)(&v)+1);
      *(((__near uint8_t*)dst) + 2) = *((uint8_t*)(&v)+2);
      *(((__near uint8_t*)dst) + 3) = *((uint8_t*)(&v)+3);
  }
}

class CEEPROM
{
private:
  
public:
  static void unlock()
  {
    if ((FLASH_IAPSR & FLASH_FLAG_DUL) != FLASH_FLAG_DUL)
    {
       FLASH_DUKR = 0xAE;
       FLASH_DUKR = 0x56;

       while ((FLASH_IAPSR & FLASH_FLAG_DUL) != FLASH_FLAG_DUL)
       {
       }
    }    
  }
  
  static void lock()
  {
    FLASH_IAPSR = (unsigned char)(~FLASH_IAPSR_DUL);
  }
};