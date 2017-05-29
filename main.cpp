#include "stdafx.h"

////////////////////////////////////////////////////////////////////////////
// ����� ������� ���������
////////////////////////////////////////////////////////////////////////////

// ������ ����� ����������� �����������
CLeds leds(__leds_comport, __leds_compin, __leds_ledmap, 
           sizeof(__leds_ledmap) / sizeof(__leds_ledmap[0]));


// ������ ����� ����������� �������
CDisplay disp(__display_segport, __display_pinmap,
              __display_compins, 
              sizeof(__display_compins) / sizeof(__display_compins[0]),
              __display_common_catodes, 
              __display_refresh, __display_blink);

// ������ ����� ����������� ����������
CKeys keys(__keys_keymap, sizeof(__keys_keymap)/sizeof(__keys_keymap[0]),
           __keys_active_state);
  
// ������ ����� ������ � ���������
CInputs inp(__inp_map, sizeof(__inp_map)/sizeof(__inp_map[0]));

// ������ ����� ������ � ����
CRelay relay(__rel_map, sizeof(__rel_map)/sizeof(__rel_map[0]),
             __rel_on_state);

int main()
{
  // �������������� ������������
  CInit::init();
    
  // ������� �� ������� �������� '---'
  disp.set("---");

  // ������ ����� ������-������
  CBody body(disp, leds, keys, inp, relay);
  
  // �������� ���� ���������  
  while(1)
  {
    inp.tick();
    keys.tick();
    leds.tick();
    disp.tick();

    body.tick();
    
    // ���������� ������� ����������� �������
    CWatchdog::set_counter(__wwdg_counter);
  }
}
