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
  void handle_continuous();
  void stop_continuous();
  void on_key_press(uint8_t keycode) override;
  void on_key_release(uint8_t keycode) override;
  uint8_t speed_step = 0;
  bool continuous_mode = false;
  uint8_t continuous_axis = 0;
  bool continuous_direction;
  uint8_t leds=0;
};

const float NumpadStepSizes[] = {0.1, 1.0, 10.0};
const float NumpadContinuousMultipliers[] = {0.25, 0.5, 1.0};
const char NumpadAxisLetters[] = {'X','Y','Z'};

const uint16_t NumpadContinuousFeeds[] = {6000, 6000, 600};

const char NumpadContinuousRunCommand[] = "M98 P\"pendant-continuous-run.g\" A\"%c\" F%u D%u";
const char NumpadContinuousStopCommand[] = "M98 P\"pendant-continuous-stop.g\"";

const char* const NumpadMoveCommands[] =
{
  "G91 G1 F6000 X",     // X axis
  "G91 G1 F6000 Y",     // Y axis
  "G91 G1 F600 Z",      // Z axis
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
  "", // Button 0_INS
  ""  // Button DOT_DEL
};

#endif