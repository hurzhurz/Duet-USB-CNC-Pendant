#ifndef USBHIDPendant_H
#define USBHIDPendant_H

#include <Arduino.h>
#include "Adafruit_TinyUSB.h"

#define MAX_PRESSED_KEYS 6

struct DuetStatus
{
  double axis_userPosition[6];
  uint16_t spindle_speed;
  double speedFactor;
};

class USBHIDPendant
{
public:
  USBHIDPendant(uint8_t dev_addr, uint8_t instance): kb_dev_addr(dev_addr), kb_instance(instance) {};
  virtual ~USBHIDPendant();
  virtual void report_received(uint8_t const *report, uint16_t len){};
  virtual void set_report_complete(uint8_t report_id, uint8_t report_type, uint16_t len){};
  virtual void duetstatus_received(DuetStatus * duetstatus){};
  virtual void loop(){};

protected:
  void set_report(uint8_t report_id, uint8_t report_type, void* report, uint16_t len);
  void send_command(String * cmd);
  void process_keycodes(const uint8_t *keycodes, uint8_t len);
  bool is_key_pressed(uint8_t keycode);
  virtual void on_key_press(uint8_t keycode){}
  virtual void on_key_release(uint8_t keycode){}

private:
  uint8_t kb_dev_addr;
  uint8_t kb_instance;
  uint8_t pressed_keys[MAX_PRESSED_KEYS]={};
  bool byte_in_bytearray(uint8_t byte, const uint8_t *array, uint8_t len);
};

struct USBHIDPendantDevice
{
  uint8_t dev_addr;
  uint8_t instance;
  USBHIDPendant *object;
};




#endif