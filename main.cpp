#include "stdafx.h"

////////////////////////////////////////////////////////////////////////////
// Точка запуска программы
////////////////////////////////////////////////////////////////////////////

// Создаём класс контроллера светодиодов
CLeds leds(__leds_comport, __leds_compin, __leds_ledmap, 
           sizeof(__leds_ledmap) / sizeof(__leds_ledmap[0]));


// Создаём класс контроллера дисплея
CDisplay disp(__display_segport, __display_pinmap,
              __display_compins, 
              sizeof(__display_compins) / sizeof(__display_compins[0]),
              __display_common_catodes, 
              __display_refresh, __display_blink);

// Создаём класс контроллера клавиатуры
CKeys keys(__keys_keymap, sizeof(__keys_keymap)/sizeof(__keys_keymap[0]),
           __keys_active_state);
  
// Создаём класс работы с датчиками
CInputs inp(__inp_map, sizeof(__inp_map)/sizeof(__inp_map[0]));

// Создаём класс работы с реле
CRelay relay(__rel_map, sizeof(__rel_map)/sizeof(__rel_map[0]),
             __rel_on_state);

int main()
{
  // Инициализируем оборудование
  CInit::init();
    
  // Выводим на дисплей значение '---'
  disp.set("---");

  // Создаём класс бизнес-логики
  CBody body(disp, leds, keys, inp, relay);
  
  // Основной цикл программы  
  while(1)
  {
    inp.tick();
    keys.tick();
    leds.tick();
    disp.tick();

    body.tick();
    
    // Сбрасываем счётчик сторожевого таймера
    CWatchdog::set_counter(__wwdg_counter);
  }
}
