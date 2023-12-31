#ifndef Pendant_NUMPAD_H
#define Pendant_NUMPAD_H

#include "USBHIDPendant.h"

#define KEYCODE_BACKSPACE 42 
#define KEYCODE_NUMLOCK 83
#define KEYCODE_DIV 84
#define KEYCODE_MULT 85
#define KEYCODE_SUB 86
#define KEYCODE_ADD 87
#define KEYCODE_ENTER 88
#define KEYCODE_1_END 89
#define KEYCODE_2_DOWN 90
#define KEYCODE_3_PGDN 91
#define KEYCODE_4_LEFT 92
#define KEYCODE_5 93
#define KEYCODE_6_RIGHT 94
#define KEYCODE_7_HOME 95
#define KEYCODE_8_UP 96
#define KEYCODE_9_PGUP 97
#define KEYCODE_0_INS 98
#define KEYCODE_DOT_DEL 99

class Pendant_Numpad: public USBHIDPendant
{
public:
  Pendant_Numpad(uint8_t dev_addr, uint8_t instance): USBHIDPendant(dev_addr, instance){};
  void report_received(uint8_t const *report, uint16_t len) override;
  void loop() override;
  void toogle_numlockled();
  void set_numlockled(bool state);
private:
  void on_key_press(uint8_t keycode) override;
  void on_key_release(uint8_t keycode) override;
  float multiplier=0.0;
  uint8_t leds=0;
};

const char* const NumpadMoveCommands[] =
{
  "G91 G0 F6000 X",     // X axis
  "G91 G0 F6000 Y",     // Y axis
  "G91 G0 F600 Z",      // Z axis
};

const char* const NumpadButtonCommands[] =
{
  "", // Button BACKSPACE
  "", // Button NUMLOCK
  "", // Button DIV
  "", // Button MULT
  "", // Button SUB
  "", // Button ADD
  "M98 P\"Numpad/enter.g\"", // Button ENTER
  "M98 P\"Numpad/1_end.g\"", // Button 1_END
  "", // Button 2_DOWN
  "M98 P\"Numpad/3_pgdn.g\"", // Button 3_PGDN
  "", // Button 4_LEFT
  "M98 P\"Numpad/5.g\"", // Button 5
  "", // Button 6_RIGHT
  "M98 P\"Numpad/7_home.g\"", // Button 7_HOME
  "", // Button 8_UP
  "M98 P\"Numpad/9_pgup.g\"", // Button 9_PGUP
  "M98 P\"Numpad/0_ins.g\"", // Button 0_INS
  "M98 P\"Numpad/dort_del.g\""  // Button DOT_DEL
};

#endif