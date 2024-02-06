#ifndef Pendant_WHB04B6_H
#define Pendant_WHB04B6_H

// For Reference:
// https://github.com/LinuxCNC/linuxcnc/tree/master/src/hal/user_comps/xhc-WHB04B6

#include "USBHIDPendant.h"

#define REPORT_INTERVAL 2000
#define CMD_STEP_INTERVAL 100
#define CMD_CONTINUOUS_CHECK_INTERVAL 100
#define CMD_CONTINUOUS_UPDATE_INTERVAL 500

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

#define AXISSELCTOR_OFF 0x06
#define AXISSELCTOR_X 0x11
#define AXISSELCTOR_Y 0x12
#define AXISSELCTOR_Z 0x13
#define AXISSELCTOR_A 0x14
#define AXISSELCTOR_B 0x15
#define AXISSELCTOR_C 0x16

#define FEEDSELECTOR_2P_0001 0x0D
#define FEEDSELECTOR_5P_001 0x0E
#define FEEDSELECTOR_10P_01 0x0F
#define FEEDSELECTOR_30P_1 0x10
#define FEEDSELECTOR_60P 0x1A
#define FEEDSELECTOR_100P 0x1B
#define FEEDSELECTOR_LEAD1 0x9B
#define FEEDSELECTOR_LEAD2 0x1C
#define FEEDSELECTOR_STEP_STEPS 4
#define FEEDSELECTOR_CONT_STEPS 6

#define FEEDSELECTOR_TO_LINEAR(x) ((x>=FEEDSELECTOR_2P_0001 && x<=FEEDSELECTOR_30P_1)?(x-FEEDSELECTOR_2P_0001+1):((x>=FEEDSELECTOR_60P&&x<=FEEDSELECTOR_100P)?(x-FEEDSELECTOR_60P+5):0))
#define FEEDSELECTOR_IS_LEAD(x) (x==FEEDSELECTOR_LEAD1 || x==FEEDSELECTOR_LEAD2)

class Pendant_WHB04B6: public USBHIDPendant
{
public:
  Pendant_WHB04B6(uint8_t dev_addr, uint8_t instance, DynamicJsonDocument* config=0);
  ~Pendant_WHB04B6();
  void report_received(uint8_t const *report, uint16_t len) override;
  void duetstatus_received(DuetStatus * duetstatus) override;
  void loop() override;
private:
  void send_display_report();
  void double_to_report_bytes(double val, uint8_t idx_intval_lower, uint8_t idx_intval_upper, uint8_t idx_frac_lower, uint8_t idx_frac_upper);
  void uint16_to_report_bytes(uint16_t val, uint8_t idx_lower, uint8_t idx_upper);
  double axis_coordinates[6];
  uint8_t display_report_data[24];
  uint8_t selected_axis, display_axis_offset, selected_feed;
  unsigned long last_display_report;
  int16_t jog;
  void on_key_press(uint8_t keycode) override;
  void on_key_release(uint8_t keycode) override;
  void handle_continuous_check();
  void handle_continuous_update();
  void stop_continuous();
  uint8_t continuous_axis = 0;
  bool continuous_direction;
  unsigned long last_continuous_check;
  unsigned long last_continuous_update;
  enum class Mode : uint8_t
  {
    Step = 0,
    Continuous
  } mode = Mode::Continuous;
};

const uint16_t WHB04B6ContinuousFeeds[] = {6000, 6000, 600, 6000, 6000, 6000};
const float WHB04B6StepSizes[] = {0.001, 0.01, 0.1, 1.0};
const float WHB04B6ContinuousMultipliers[] = {0.02, 0.05, 0.10, 0.30, 0.60, 1.00};
const char WHB04B6AxisLetters[] = {'X','Y','Z','U','V','W'};
const uint8_t WHB04B6AxisCount = 6;

const char* const WHB04B6MoveCommands[] =
{
  "G91 G1 F6000 X",     // X axis
  "G91 G1 F6000 Y",     // Y axis
  "G91 G1 F600 Z",      // Z axis
  "G91 G1 F6000 U",      // axis 4
  "G91 G1 F6000 V",      // axis 5
  "G91 G1 F6000 W",      // axis 6
};

const char WHB04B6ContinuousRunCommand[] = "M98 P\"pendant-continuous-run.g\" A\"%c\" F%u D%u";
const char WHB04B6ContinuousStopCommand[] = "M98 P\"pendant-continuous-stop.g\"";

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