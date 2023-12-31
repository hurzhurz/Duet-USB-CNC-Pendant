#include <Arduino.h>

#include <ArduinoJson.h>

#include <GCodeSerial.h>
#include <PassThrough.h>
#include "JSONReader.h"
#include "USBHIDPendant.h"

#define DuetSerial Serial1 // UART0
#define DuetSerialTXPin 12
#define DuetSerialRXPin 13
#define DuetSerialBaud 57600

#define PanelDueSerial Serial2 // UART1
#define PanelDueSerialTXPin 4
#define PanelDueSerialRXPin 5
#define PanelDueSerialBaud 57600

#define DUET_QUERY_INTERVAL 1000

GCodeSerial output(DuetSerial);
//GCodeSerial output((HardwareSerial&)Serial);
PassThrough passThrough;
JSONReader jsonReader;
StaticJsonDocument<1000> jsondoc;
StaticJsonDocument<200> jsonfilter;

void setup()
{
  Serial.begin(115200); // USB Serial for debug output

  jsonfilter["result"]["move"]["axes"] = true;
  jsonfilter["result"]["spindles"][0]["current"] = true;


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
  static unsigned long last_duet_query = millis();
  unsigned long now = millis();
  if((now-last_duet_query) > DUET_QUERY_INTERVAL)
  {
    last_duet_query = now;
    output.print("M409 F\"d99f\"\n");
  }

  // check if complete JSON status message from Duet is ready in buffer
  if(jsonReader.isReady())
  {
    char *jsonbuffer = jsonReader.getBuffer();
    Serial.println("Got complete JSON message");

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
        // extract selected data from JSON message and forward to USB routine on other core
        DuetStatus * duetstatus = new DuetStatus;
        for(uint8_t i=0;i<6;i++)
          duetstatus->axis_userPosition[i] = (double)jsondoc["result"]["move"]["axes"][i]["userPosition"];
          duetstatus->spindle_speed = (uint16_t)jsondoc["result"]["spindles"][0]["current"];
        rp2040.fifo.push_nb((uint32_t)duetstatus);
    }
    
  }

}