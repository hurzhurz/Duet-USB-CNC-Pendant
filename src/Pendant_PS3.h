#ifndef Pendant_PS3_H
#define Pendant_PS3_H

#include "USBHIDPendant.h"

#define BUTTON_SELECT 0x00
#define BUTTON_LEFT_STICK 0x01
#define BUTTON_RIGHT_STICK 0x02
#define BUTTON_START 0x03
#define BUTTON_UP 0x04
#define BUTTON_RIGHT 0x05
#define BUTTON_DOWN 0x06
#define BUTTON_LEFT 0x07
#define BUTTON_LEFT_TRIGGER 0x08
#define BUTTON_RIGHT_TRIGGER 0x09
#define BUTTON_LEFT_SHOULDER 0x0a
#define BUTTON_RIGHT_SHOULDER 0x0b
#define BUTTON_TRIANGLE 0x0c
#define BUTTON_CIRCLE 0x0d
#define BUTTON_X 0x0e
#define BUTTON_SQUARE 0x0f
#define BUTTON_PS 0x10


// https://github.com/felis/USB_Host_Shield_2.0/blob/master/PS3Enums.h
/** Report buffer for all PS3 commands */
#define PS3_REPORT_BUFFER_SIZE  48
const uint8_t PS3_REPORT_BUFFER[PS3_REPORT_BUFFER_SIZE] = {
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x27, 0x10, 0x00, 0x32,
        0xff, 0x27, 0x10, 0x00, 0x32,
        0xff, 0x27, 0x10, 0x00, 0x32,
        0xff, 0x27, 0x10, 0x00, 0x32,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


class Pendant_PS3: public USBHIDPendant
{
public:
  Pendant_PS3(uint8_t dev_addr, uint8_t instance);
  void report_received(uint8_t const *report, uint16_t len) override;
  void loop() override;

private:
  void process_buttons(const uint8_t *buttonbytebegin);
  bool is_button_pressed(uint8_t button);
  void on_key_press(uint8_t keycode) override;
  void on_key_release(uint8_t keycode) override;
  void set_leds(uint8_t leds);
  uint32_t pressed_buttons=0;
  uint8_t step=0;
  uint8_t write_buffer[PS3_REPORT_BUFFER_SIZE];
};

const char* const PS3MoveCommands[] =
{
  "G91 G1 F6000 X",     // X axis
  "G91 G1 F6000 Y",     // Y axis
  "G91 G1 F600 Z",      // Z axis
};

const char* const PS3ButtonCommands[] =
{
  "M98 P\"PS3/select.g\"", // Button SELECT
  "M98 P\"PS3/left-stick.g\"", // Button LEFT_STICK
  "M98 P\"PS3/right-stick.g\"", // Button RIGHT_STICK
  "M98 P\"PS3/start.g\"", // Button START
  "", // Button UP
  "", // Button RIGHT
  "", // Button DOWN
  "", // Button LEFT
  "", // Button LEFT_TRIGGER
  "", // Button RIGHT_TRIGGER
  "", // Button LEFT_SHOULDER
  "", // Button RIGHT_SHOULDER
  "M98 P\"PS3/triangle.g\"", // Button TRIANGLE
  "M98 P\"PS3/circle.g\"", // Button CIRCLE
  "M98 P\"PS3/x.g\"", // Button X
  "M98 P\"PS3/square.g\"", // Button SQUARE
  "M98 P\"PS3/ps.g\"" // Button PS
};



#endif