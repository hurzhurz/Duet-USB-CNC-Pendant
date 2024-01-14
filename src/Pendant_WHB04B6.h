#ifndef Pendant_WHB04B6_H
#define Pendant_WHB04B6_H

// For Reference:
// https://github.com/LinuxCNC/linuxcnc/tree/master/src/hal/user_comps/xhc-WHB04B6

#include "USBHIDPendant.h"

#define REPORT_INTERVAL 2000
#define CMD_INTERVAL 20

#define SEED 0xff

#define R_IDX(x) (x+1+(x/7)) // helper to accommodate report package header for relative offset

#define KEYCODE_RESET 0x01
#define KEYCODE_STOP 0x02
#define KEYCODE_STARTPAUSE 0x03
#define KEYCODE_M1_FEEDPLUS 0x04
#define KEYCODE_M2_FEEDMINUS 0x05
#define KEYCODE_M3_SPINDLEPLUS 0x06
#define KEYCODE_M4_SPINDLEMINUS 0x07
#define KEYCODE_M5_MHOME 0x08
#define KEYCODE_M6_SAFEZ 0x09
#define KEYCODE_M7_WHOME 0x0a
#define KEYCODE_M8_SPINDLEONOFF 0x0b
#define KEYCODE_FN 0x0c
#define KEYCODE_M9_PROBEZ 0x0d
#define KEYCODE_CONTINUOUS 0x0e
#define KEYCODE_STEP 0x0f
#define KEYCODE_M10 0x10

class Pendant_WHB04B6: public USBHIDPendant
{
public:
  Pendant_WHB04B6(uint8_t dev_addr, uint8_t instance);
  void report_received(uint8_t const *report, uint16_t len) override;
  void duetstatus_received(DuetStatus * duetstatus) override;
  void loop() override;
private:
  void send_display_report();
  void double_to_report_bytes(double val, uint8_t idx_intval_lower, uint8_t idx_intval_upper, uint8_t idx_frac_lower, uint8_t idx_frac_upper);
  void uint16_to_report_bytes(uint16_t val, uint8_t idx_lower, uint8_t idx_upper);
  double axis_coordinates[6];
  uint8_t display_report_data[24];
  uint8_t selected_axis, display_axis_offset, selected_feed, mode;
  unsigned long last_display_report;
  int16_t jog;
  void on_key_press(uint8_t keycode) override;
  void on_key_release(uint8_t keycode) override;
};


const char* const WHB04B6MoveCommands[] =
{
  "G91 G1 F6000 X",     // X axis
  "G91 G1 F6000 Y",     // Y axis
  "G91 G1 F600 Z",      // Z axis
  "G91 G1 F6000 U",      // axis 4
  "G91 G1 F6000 V",      // axis 5
  "G91 G1 F6000 W",      // axis 6
};

const char* const WHB04B6ButtonCommands[] =
{
  "M98 P\"WHB04B6/reset.g\"", // Button RESET
  "M98 P\"WHB04B6/stop.g\"", // Button STOP
  "M98 P\"WHB04B6/start-pause.g\"", // Button STARTPAUSE
  "M98 P\"WHB04B6/macro-1.g\"", // Button M1_FEEDPLUS
  "M98 P\"WHB04B6/macro-2.g\"", // Button M2_FEEDMINUS
  "M98 P\"WHB04B6/macro-3.g\"", // Button M3_SPINDLEPLUS
  "M98 P\"WHB04B6/macro-4.g\"", // Button M4_SPINDLEMINUS
  "M98 P\"WHB04B6/macro-5.g\"", // Button M5_MHOME
  "M98 P\"WHB04B6/macro-6.g\"", // Button M6_SAFEZ
  "M98 P\"WHB04B6/macro-7.g\"", // Button M7_WHOME
  "M98 P\"WHB04B6/macro-8.g\"", // Button M8_SPINDLEONOFF
  "", // Button FN
  "M98 P\"WHB04B6/macro-9.g\"", // Button M9_PROBEZ
  "", // Button CONTINUOUS
  "", // Button STEP
  "M98 P\"WHB04B6/macro-10.g\"", // Button M10
};

const char* const WHB04B6ButtonCommandsFN[] =
{
  "", // Button RESET
  "", // Button STOP
  "", // Button STARTPAUSE
  "M98 P\"WHB04B6/feed-plus.g\"", // Button M1_FEEDPLUS
  "M98 P\"WHB04B6/feed-minus.g\"", // Button M2_FEEDMINUS
  "M98 P\"WHB04B6/spindle-plus.g\"", // Button M3_SPINDLEPLUS
  "M98 P\"WHB04B6/spindle-minus.g\"", // Button M4_SPINDLEMINUS
  "M98 P\"WHB04B6/m-home.g\"", // Button M5_MHOME
  "M98 P\"WHB04B6/safe-z.g\"", // Button M6_SAFEZ
  "M98 P\"WHB04B6/w-home.g\"", // Button M7_WHOME
  "M98 P\"WHB04B6/spindle-onoff.g\"", // Button M8_SPINDLEONOFF
  "", // Button FN
  "M98 P\"WHB04B6/probe-z.g\"", // Button M9_PROBEZ
  "", // Button CONTINUOUS
  "", // Button STEP
  "", // Button M10
};

#endif