////////////////////////////////////////////////////////////////////////////
// �����, ����������� ������������� ��������� ����������������
////////////////////////////////////////////////////////////////////////////
class CInit
{
private:
  // ��������� ������������ ����������������
  static void init_clk_hsi()
  {
    // ��������� ������ HSE
    CLK_ECKR_bit.HSEEN = 0;

    // ��������� ������������ �����������
    CLK_SWCR_bit.SWEN = 1;
    
    // �������� ������� (prescalers) ��� �������� ��������� = 1
    // ���� �������� �� ������� HSE
    CLK_CKDIVR = 0;   
  }
  
  static bool init_clk_hse()
  {
    // ��������� ������ HSE (high-speed external) ����������
    // HSE - ��������� � ������� �������
    CLK_ECKR_bit.HSEEN = 1;
    
    // ��������� ������������ �����������
    CLK_SWCR_bit.SWEN = 1;
    
    // ������� ���������� ��������� ����������
    long long hse_ready_count = 10000;
    
    while (CLK_ECKR_bit.HSERDY != 1 && hse_ready_count > 0)
      hse_ready_count--;
    
    if (!hse_ready_count) return false;

    // �������� ������� (prescalers) ��� �������� ��������� = 1
    // ���� �������� �� ������� HSE
    CLK_CKDIVR = 0;
    
    // ������������� �������� �������� ��������� - HSE
    CLK_SWR = 0xB4;
        
    // �������� ������� �������
    while (CLK_SWCR_bit.SWIF != 1);
    
    // ��������� �������������� ������������ �� ���������� ��������� (HSI)
    // � ������ ������������� ��������
    CLK_CSSR_bit.CSSEN = 1;
    
    return true;
  }
    
public:
  static void init()
  {
    // Configure system clock
    if (!init_clk_hse())
      init_clk_hsi();
   
    // ����������� � ��������� watchdog �� ���� 50 ����
    CWatchdog::Init(__wwdg_counter, 0x7f);
    
    // ��������� ����������
    asm("rim");
  }
};
