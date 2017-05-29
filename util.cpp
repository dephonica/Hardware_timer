class CUtil
{
private:
  
public:
  inline static uint8_t toupper(uint8_t c)
  {
    if (c >= 'a' && c <= 'z') 
      c -= 'a' - 'A';
    
    return c;
  }
};
