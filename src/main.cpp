#include <Arduino.h>

#include <ArduinoJson.h>

#include <GCodeSerial.h>
#include <PassThrough.h>
#include "JSONReader.h"
#include "USBHIDPendant.h"

#define DUET_RESPONSEKEY_SPEEDFACTOR "move.speedFactor"
#define DUET_RESPONSEKEY_SPINDLESPEED "spindles[0].current"
#define DUET_RESPONSEKEY_POSITION "move.axes[].userPosition"

#define DUET_STATUS_REQUEST_CMD_POSITION "M409 K\"move.axes[].userPosition\"\n"
#define DUET_STATUS_REQUEST_CMD_OTHER "M409 K\"move.speedFactor\" M409 K\"spindles[0].current\"\n"
#define DUET_STATUS_REQUEST_INTERVAL_POSITION 500
#define DUET_STATUS_REQUEST_INTERVAL_OTHER 1000

#define DuetSerial Serial1 // UART0
#define DuetSerialTXPin 12
#define DuetSerialRXPin 13
#define DuetSerialBaud 57600

#define PanelDueSerial Serial2 // UART1
#define PanelDueSerialTXPin 4
#define PanelDueSerialRXPin 5
#define PanelDueSerialBaud 57600



GCodeSerial output(DuetSerial);
//GCodeSerial output((HardwareSerial&)Serial);
PassThrough passThrough;

JSONReader jsonReader(200, "{\"key\":\"", "{\"key\":\"\""); // expect non empty key, want '{"key":"' but not '{"key":""'
StaticJsonDocument<1000> jsondoc;
StaticJsonDocument<100> jsonfilter;
DuetStatus duetstatus = {};

void setup()
{
  Serial.begin(115200); // USB Serial for debug output
  jsonfilter["key"] = true;
  jsonfilter["result"] = true;


  DuetSerial.setTX(DuetSerialTXPin);
  DuetSerial.setRX(DuetSerialRXPin);

  PanelDueSerial.setTX(PanelDueSerialTXPin);
  PanelDueSerial.setRX(PanelDueSerialRXPin);

  
  //while ( !Serial ) delay(10);   // wait for native usb

  Serial.println("TinyUSB Dual Device Info Example");

  // Send ready state and wait for other core
  Serial.println("Core0 Ready");
  rp2040.fifo.push(0);
  rp2040.fifo.pop();
  Serial.println("Core0 Start");

  DuetSerial.begin(DuetSerialBaud);
  PanelDueSerial.begin(PanelDueSerialBaud);
}


void loop()
{
  // check for G-Code command from USB routine on other core
  while(rp2040.fifo.available())
  {
    String * cmd;
    cmd = (String*)rp2040.fifo.pop();
    output.print(*cmd);
    //Serial.print(*cmd);
    delete cmd;
  }

  // read serial from Duet, forward to PanelDue and buffer/read JSON status data
  while(DuetSerial.available())
  {
    uint8_t c=DuetSerial.read();
    PanelDueSerial.write(c);
    jsonReader.write(c);
  }
  
  // read serial from PanelDue and handle passthrough to Duet
  unsigned int commandLength = passThrough.Check(PanelDueSerial);
  if (commandLength != 0)
  {
    const char * cmd = passThrough.GetCommand();
    uint8_t start = 0;
    if(cmd[0]=='N') // prevent duplicate line number in line
    {
        for(uint8_t i=1;i<commandLength;i++)
        {
            if(cmd[i]<0x30 || cmd[i]>0x39) // check if still number
            {
              start=i+1;
              break;
            }
        }
    }
    output.write(&(cmd[start]), commandLength-start);
  }

  // request status from Duet
  unsigned long now = millis();
  static unsigned long last_duet_status_request_position = now;
  if((now-last_duet_status_request_position) > DUET_STATUS_REQUEST_INTERVAL_POSITION)
  {
    last_duet_status_request_position = now;
    output.print(DUET_STATUS_REQUEST_CMD_POSITION);
  }
  static unsigned long last_duet_status_request_other = now;
  if((now-last_duet_status_request_other) > DUET_STATUS_REQUEST_INTERVAL_OTHER)
  {
    last_duet_status_request_other = now;
    output.print(DUET_STATUS_REQUEST_CMD_OTHER);
  }
  if(jsonReader.isReady())
  {
    char *jsonbuffer = jsonReader.getBuffer();
    Serial.print("Got complete JSON message: ");

    // try to parse JSON
    DeserializationError error = deserializeJson(jsondoc, jsonbuffer, DeserializationOption::Filter(jsonfilter));

    // Test if parsing succeeds.
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
    }
    else
    {
        Serial.println((const char*)jsondoc["key"]);
        // extract selected data from JSON message
        const char* key = jsondoc["key"];
        if(strcmp(key, DUET_RESPONSEKEY_SPEEDFACTOR) == 0)
        {
          duetstatus.speedFactor = (double)jsondoc["result"];
        }
        if(strcmp(key, DUET_RESPONSEKEY_SPINDLESPEED) == 0)
        {
          duetstatus.spindle_speed = (uint16_t)jsondoc["result"];
        }
        if(strcmp(key, DUET_RESPONSEKEY_POSITION) == 0)
        {
          for(uint8_t i=0;i<6;i++)
            duetstatus.axis_userPosition[i] = (double)jsondoc["result"][i];
          // forward current status to USB routine on other core
          rp2040.fifo.push_nb((uint32_t)new DuetStatus(duetstatus));
        }
    }
  }

}