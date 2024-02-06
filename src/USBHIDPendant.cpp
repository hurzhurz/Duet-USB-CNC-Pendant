#include "USBHIDPendant.h"



void USBHIDPendant::set_report(uint8_t report_id, uint8_t report_type, void* report, uint16_t len)
{
    tuh_hid_set_report(this->kb_dev_addr, this->kb_instance, report_id, report_type, report, len);
}


void USBHIDPendant::process_keycodes(const uint8_t *keycodes, uint8_t len)
{
  uint8_t to_release[MAX_PRESSED_KEYS]={}, to_release_count=0;
  uint8_t to_press[MAX_PRESSED_KEYS]={}, to_press_count=0;
  uint8_t i;

  for(i=0; i<len; i++)
    if(keycodes[i] && !this->byte_in_bytearray(keycodes[i], this->pressed_keys, MAX_PRESSED_KEYS))
      to_press[to_press_count++]=keycodes[i];
  for(i=0; i<MAX_PRESSED_KEYS; i++)
    if(this->pressed_keys[i] && !this->byte_in_bytearray(this->pressed_keys[i], keycodes, len))
      to_release[to_release_count++]=this->pressed_keys[i];

  for(i=0; i<MAX_PRESSED_KEYS; i++)
    this->pressed_keys[i] = (i<len)?keycodes[i]:0;

  for(i=0; i<to_release_count; i++)
    this->on_key_release(to_release[i]);
  for(i=0; i<to_press_count; i++)
    this->on_key_press(to_press[i]);
}

bool USBHIDPendant::byte_in_bytearray(uint8_t byte, const uint8_t *array, uint8_t len)
{
  for(uint8_t i=0; i<len; i++)
    if(array[i]==byte)
      return true;
  return false;
}

bool USBHIDPendant::is_key_pressed(uint8_t keycode)
{
  return this->byte_in_bytearray(keycode, this->pressed_keys, MAX_PRESSED_KEYS);
}

USBHIDPendant::~USBHIDPendant()
{
  // Release all Keys
  for(uint8_t i=0; i<MAX_PRESSED_KEYS; i++)
    if(this->pressed_keys[i])
      this->on_key_release(this->pressed_keys[i]);
  // Run Loop a last time
  this->loop();
  delete config;
}

void USBHIDPendant::send_command(String * cmd)
{
  cmd->concat("\n");
  rp2040.fifo.push_nb((uint32_t)cmd);
}