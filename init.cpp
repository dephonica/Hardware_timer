////////////////////////////////////////////////////////////////////////////
// Класс, реализующий инициализацию устройств микроконтроллера
////////////////////////////////////////////////////////////////////////////
class CInit
{
private:
  // Настройка тактирования микроконтроллера
  static void init_clk_hsi()
  {
    // Запрещаем работу HSE
    CLK_ECKR_bit.HSEEN = 0;

    // Разрешаем переключение генераторов
    CLK_SWCR_bit.SWEN = 1;
    
    // Делители частоты (prescalers) для тактовых импульсов = 1
    // Ядро работает на частоте HSE
    CLK_CKDIVR = 0;   
  }
  
  static bool init_clk_hse()
  {
    // Разрешаем работу HSE (high-speed external) генератора
    // HSE - генератор с внешним кварцем
    CLK_ECKR_bit.HSEEN = 1;
    
    // Разрешаем переключение генераторов
    CLK_SWCR_bit.SWEN = 1;
    
    // Ожидаем готовности тактового генератора
    long long hse_ready_count = 10000;
    
    while (CLK_ECKR_bit.HSERDY != 1 && hse_ready_count > 0)
      hse_ready_count--;
    
    if (!hse_ready_count) return false;

    // Делители частоты (prescalers) для тактовых импульсов = 1
    // Ядро работает на частоте HSE
    CLK_CKDIVR = 0;
    
    // Устанавливаем источник тактовых импульсов - HSE
    CLK_SWR = 0xB4;
        
    // Ожидание захвата частоты
    while (CLK_SWCR_bit.SWIF != 1);
    
    // Разрешаем автоматическое переключение на внутренний генератор (HSI)
    // в случае неисправности внешнего
    CLK_CSSR_bit.CSSEN = 1;
    
    return true;
  }
    
public:
  static void init()
  {
    // Configure system clock
    if (!init_clk_hse())
      init_clk_hsi();
   
    // Настраиваем и запускаем watchdog на цикл 50 мсек
    CWatchdog::Init(__wwdg_counter, 0x7f);
    
    // Разрешаем прерывания
    asm("rim");
  }
};
