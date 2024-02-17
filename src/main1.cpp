#include <Arduino.h>

#include "USBHIDPendant.h"
#include "Pendant_Numpad.h"
#include "Pendant_WHB04B6.h"
#include "Pendant_PS3.h"


// Example used as base for USB HID stuff:
// https://github.com/adafruit/Adafruit_TinyUSB_Arduino/tree/master/examples/DualRole/HID/hid_device_report

/*********************************************************************
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 Copyright (c) 2019 Ha Thach for Adafruit Industries
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

#include <LittleFS.h>
#include <SingleFileDrive.h>
#include <ArduinoJson.h>
#include <ArduinoYaml.h>

DynamicJsonDocument config_json_doc(4096);

// pio-usb is required for rp2040 host
#include "pio_usb.h"
#define HOST_PIN_DP   16   // GPIO Pin used as D+ for host, D- = D+ + 1

#include "Adafruit_TinyUSB.h"

#define LANGUAGE_ID 0x0409  // English

// USB Host object
Adafruit_USBH_Host USBHost;

// holding device descriptor
tusb_desc_device_t desc_device;

#define MOUNT_CHECK_INTERVAL 1000
#define MAX_DEV 5
struct USBHIDPendantDevice devices[MAX_DEV];


DynamicJsonDocument * get_config(const char * devicetype, uint16_t vid, uint16_t pid);

// the setup function runs once when you press reset or power the board

// core1's setup
void setup1() {
  //while ( !Serial ) delay(10);   // wait for native usb
  Serial.println("Core1 setup to run TinyUSB host with pio-usb");

  LittleFS.begin();
  singleFileDrive.begin("config.yml", "READONLY_config.yml");

  File file = LittleFS.open( "config.yml", "r" );
  if (!file)
  {
    Serial.println("config file open failed");
  }
  else
  {
    DeserializationError error = deserializeYml( config_json_doc, file ); // convert yaml to json
    file.close();
    if (error)
    {
        Serial.print(F("deserializeYml() failed: "));
        Serial.println(error.f_str());
    }
  }


  // Check for CPU frequency, must be multiple of 120Mhz for bit-banging USB
  uint32_t cpu_hz = clock_get_hz(clk_sys);
  if ( cpu_hz != 120000000UL && cpu_hz != 240000000UL ) {
    while ( !Serial ) delay(10);   // wait for native usb
    Serial.printf("Error: CPU Clock = %u, PIO USB require CPU clock must be multiple of 120 Mhz\r\n", cpu_hz);
    Serial.printf("Change your CPU Clock to either 120 or 240 Mhz in Menu->CPU Speed \r\n", cpu_hz);
    while(1) delay(1);
  }

  pio_usb_configuration_t pio_cfg = PIO_USB_DEFAULT_CONFIG;
  pio_cfg.pin_dp = HOST_PIN_DP;
  USBHost.configure_pio_usb(1, &pio_cfg);
  
  for(uint8_t i = 0 ; i < MAX_DEV ; i++)
  {
    devices[i].dev_addr = 0;
    devices[i].instance = 0;
    devices[i].object = 0;
  }
  // run host stack on controller (rhport) 1
  // Note: For rp2040 pico-pio-usb, calling USBHost.begin() on core1 will have most of the
  // host bit-banging processing works done in core1 to free up core0 for other works

  // Send ready state and wait for other core
  Serial.println("Core1 Ready");
  rp2040.fifo.push(0);
  rp2040.fifo.pop();
  Serial.println("Core1 Start");

  USBHost.begin(1);
}

void check_devices_still_mounted()
{
  for(uint8_t i = 0; i< MAX_DEV; i++)
  {
    if(devices[i].object)
    {
      if (!tuh_hid_mounted(devices[i].dev_addr, devices[i].instance))
      {
        Serial.printf("HID device address = %d, instance = %d no more mounted\r\n", devices[i].dev_addr, devices[i].instance);
        delete devices[i].object;
        devices[i].object = 0;
      }
    }
  }
}

// core1's loop
void loop1()
{
  USBHost.task();

  static unsigned long last_mount_check = millis();
  unsigned long now=millis();
  if((now-last_mount_check)>MOUNT_CHECK_INTERVAL)
  {
    last_mount_check = millis();
    check_devices_still_mounted();
  }

  // check for new Duet status messages from Serial routines on other core
  DuetStatus * duetstatus = 0;
  if(rp2040.fifo.available())
    duetstatus = (DuetStatus*)rp2040.fifo.pop();

  // loop through devices, forward Duet status messages and call loop function
  for(uint8_t i = 0; i< MAX_DEV; i++)
  {
    if(devices[i].object)
    {
      if(duetstatus)
        devices[i].object->duetstatus_received(duetstatus);
      devices[i].object->loop();
    }
  }

  delete duetstatus;
}

// Invoked when device with hid interface is mounted
// Report descriptor is also available for use.
// tuh_hid_parse_report_descriptor() can be used to parse common/simple enough
// descriptor. Note: if report descriptor length > CFG_TUH_ENUMERATION_BUFSIZE,
// it will be skipped therefore report_desc = NULL, desc_len = 0
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *desc_report, uint16_t desc_len) {
  (void)desc_report;
  (void)desc_len;
  uint16_t vid, pid;

  // check if already mounted, immitate unmount first if it is
  for(uint8_t i = 0 ; i < MAX_DEV ; i++)
  {
    if(devices[i].dev_addr == dev_addr && devices[i].instance == instance && devices[i].object)
    {
      Serial.printf("HID device address = %d, instance = %d was already mounted, treat as unmounted first\r\n", dev_addr, instance);
      delete devices[i].object;
      devices[i].object = 0;
    }
  }

  tuh_vid_pid_get(dev_addr, &vid, &pid);

  Serial.printf("HID device address = %d, instance = %d is mounted\r\n", dev_addr, instance);
  Serial.printf("VID = %04x, PID = %04x\r\n", vid, pid);
  if (!tuh_hid_receive_report(dev_addr, instance)) {
    Serial.printf("Error: cannot request to receive report\r\n");
    return;
  }
  // Interface protocol (hid_interface_protocol_enum_t)
  const char* protocol_str[] = { "None", "Keyboard", "Mouse" };
  uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);
  Serial.printf("HID Interface Protocol = %s(%d)\r\n", protocol_str[itf_protocol], itf_protocol);

  // loop though device object slots
  for(uint8_t i = 0 ; i < MAX_DEV ; i++)
  {
    // search a free slot
    if(!devices[i].object)
    {
      // prepare slot
      devices[i].dev_addr = dev_addr;
      devices[i].instance = instance;
      USBHIDPendant * object = 0;
      DynamicJsonDocument * config = 0;
      // check if known device type and create matching object
      //
      // Keyboard / Numpad
      if(itf_protocol == HID_ITF_PROTOCOL_KEYBOARD)
      {
        Serial.printf("Found new Keyboard/Numpad device\r\n");
        config = get_config("Numpad", vid, pid);
        object = new Pendant_Numpad(dev_addr, instance, config);
      }
      // WHB04B-6 Wireless CNC Pendant
      // https://github.com/LinuxCNC/linuxcnc/tree/master/src/hal/user_comps/xhc-whb04b-6
      if(itf_protocol == HID_ITF_PROTOCOL_NONE && vid == 0x10ce && pid == 0xeb93)
      {
        Serial.printf("Found new WHB04B-6 device\r\n");
        config = get_config("WHB04B-6", vid, pid);
        object = new Pendant_WHB04B6(dev_addr, instance, config);
      }
      // PS3 Dualshock Controller
      if(itf_protocol == HID_ITF_PROTOCOL_NONE && vid == 0x054c && pid == 0x0268)
      {
        Serial.printf("Found new PS3 device\r\n");
        config = get_config("PS3", vid, pid);
        object = new Pendant_PS3(dev_addr, instance, config);
      }
      // store object and done
      devices[i].object = object;
      break;
    }
  }

}

// Invoked when device with hid interface is un-mounted
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
  Serial.printf("HID device address = %d, instance = %d is unmounted\r\n", dev_addr, instance);

  // check if object exists for device and destroy it
  for(uint8_t i = 0 ; i < MAX_DEV ; i++)
  {
    if(devices[i].dev_addr == dev_addr && devices[i].instance == instance && devices[i].object)
    {
      delete devices[i].object;
      devices[i].object = 0;
    }
  }
}

// Invoked when received report from device via interrupt endpoint
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len) {
  Serial.printf("HIDreport from device address = %d, instance = %d : ", dev_addr, instance);
  for (uint16_t i = 0; i < len; i++) {
    Serial.printf("0x%02X ", report[i]);
  }
  Serial.println();
  // continue to request to receive report
  if (!tuh_hid_receive_report(dev_addr, instance)) {
    Serial.printf("Error: cannot request to receive report\r\n");
  }

  // check if object exists for device and pass report
  for(uint8_t i = 0 ; i < MAX_DEV ; i++)
  {
    if(devices[i].dev_addr == dev_addr && devices[i].instance == instance && devices[i].object)
    {
      devices[i].object->report_received(report, len);
    }
  }
}

//DynamicJsonDocument *
void merge_json(JsonVariant dst, JsonVariantConst src);
DynamicJsonDocument * get_config(const char * devicetype, uint16_t vid, uint16_t pid)
{
 char name[50];
 sprintf(name, "%s_VID%04X_PID%04X", devicetype, vid, pid);
 Serial.print("Load and merge config: default, ");
 Serial.print(devicetype);
 Serial.print(", ");
 Serial.println(name);
 DynamicJsonDocument * config = new DynamicJsonDocument(2048);
 merge_json(*config, config_json_doc["default"]);
 merge_json(*config, config_json_doc[devicetype]);
 merge_json(*config, config_json_doc[name]);
 return config;
}

// https://arduinojson.org/v6/how-to/merge-json-objects/
void merge_json(JsonVariant dst, JsonVariantConst src)
{
  if(src.isNull()) return;
  if (src.is<JsonObjectConst>())
  {
    for (JsonPairConst kvp : src.as<JsonObjectConst>())
    {
      if (dst.containsKey(kvp.key())) 
        merge_json(dst[kvp.key()], kvp.value());
      else
        dst[kvp.key()] = kvp.value();
    }
  }
  else
  {
    dst.set(src);
  }
}