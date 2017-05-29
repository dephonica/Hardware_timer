#include <iostm8s105k4.h>
#include <string.h>

typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;

// ��������� �������� ����������� �������
const unsigned char __wwdg_counter = 0x7f;

/////////////////////////////////////////////////////////////////////////////
// ��������� ��������������� ����������
//
//         AAAA
//        F    B
//        F    B
//         GGGG
//        E    C
//        E    C
//         DDDD
//
// ��������� ������������ ������� ����� ��������� ����������
// �������� � ������� __display_pinmap � ������� ����������� A-G
const uint8_t __display_pinmap[7] = {5,7,2,3,4,6,1};

// ����� �����, � �������� ���������� �������� �������
const char __display_segport = 'C';

// ��������� ������������ ������� ����� �������� �������
const uint8_t __display_compins[6] = {'B',1, 'E',5, 'B',0};

// ��������� ���� ������� - ����� ����� (true), ����� ���� (false)
const bool __display_common_catodes = false;

// ������ ���������� �������, ����
const int __display_refresh = 5;

// ������ �������� �������, 
const int __display_blink = 300;

/////////////////////////////////////////////////////////////////////////////
// ��������� �������� �����������
//

// ����� ����� ������ ������ �����������
const char __leds_comport = 'B';

// ����� ������ ������ �����������
const char __leds_compin = 5;

// ����� � ������ ����������� �����������
const char __leds_ledmap[] = {'B',2, 'B',3, 'B',4};

// ������ ��� �����������, ����.
const int __leds_refresh = 3;

// �������� ������ ������� ����������, ����.
const int __leds_blink = 150;

/////////////////////////////////////////////////////////////////////////////
// ��������� �������� ����������
//

// ��������� ������������ ������ � �������
const uint8_t __keys_keymap[] = {'A',1, 'A',2};

// �������� ������� �� ���� (��� ������� �������)
bool __keys_active_state = false;

// ����� �������� ���������� ������� ��� ��������� ������� �������, ����.
const int __keys_repeat_wait = 500;

// ������ ���������� �������, ����.
const int __keys_repeat_time = 50;

// ���������� �������� ������ ����� � ����
const uint8_t __keys_keyup = 0;
const uint8_t __keys_keydown = 1;

/////////////////////////////////////////////////////////////////////////////
// ��������� �������� ��������
//

// ������ �������� ��������
#define INPUTS_DETECTOR_VERSION     2

// ����� � �������� ��������
const uint8_t __inp_map[] = {'D',0, 'D',1, 'D',2};

// ������ ������ ������ ��������, ����.
const int __in_read_period = 3;

// V1. ����� �������������� ������ ��������, ����.
// V2. ������������ �������������� ��������� ��������, ����.
const int __in_ref_period = 100;

// V1. ������������ �� ����� �������������� ��� ����������� ����� ��� ���������
// V2. ����� ������������ ����� ��� ���������.
uint16_t __in_count_active = 1000;

// V1. �� ������������
// V2. ����� ������������ ����� ��� �� ���������
uint16_t __in_count_inactive = 100;

// V1. �� ������������
// V2. ������ �������� ������� ��������-���������
const uint16_t __in_count_upper_delta = 5;

/////////////////////////////////////////////////////////////////////////////
// ��������� �������� ����
//

// ����� � �������� ����
const uint8_t __rel_map[] = {'D',3, 'D',4, 'D',5, 'D',6, 'D',7};

// ������� ������� ���������� ����
bool __rel_on_state = true;

const uint8_t __rel1_o = 0;
const uint8_t __rel2_z = 1;
const uint8_t __rel_v1 = 2;
const uint8_t __rel_v2 = 3;
const uint8_t __rel_v3 = 4;

/////////////////////////////////////////////////////////////////////////////
// ��������� ����������
//

// ����� ������� �� ��� ������� ��� ����� � ����� ����������������, ����
const int __menu_press_time = 900;

// ����� ������� �� ������� "�����" ��� ����� � ����� ��������� ��������, ����
const int __adv_menu_press_time = 15000;

// �����, ����� ������� ����������� ����� ���������������� ��� �������
// �� �������, ����
const int __menu_wait_time = 5000;

/////////////////////////////////////////////////////////////////////////////
#include "util.cpp"
#include "gpio.cpp"
#include "timer.cpp"
#include "keys.cpp"
#include "leds.cpp"
#include "display.cpp"
#include "eeprom.cpp"
#include "watchdog.cpp"

#if (INPUTS_DETECTOR_VERSION == 1)
    #include "inputs.cpp"
#else
    #include "inputs2.cpp"
#endif

#include "relay.cpp"
#include "body.cpp"
#include "init.cpp"
