#include "Pendant_PS3.h"


Pendant_PS3::Pendant_PS3(uint8_t dev_addr, uint8_t instance): USBHIDPendant(dev_addr, instance)
{
  for(uint8_t i = 0; i<PS3_REPORT_BUFFER_SIZE;i++)
    this->write_buffer[i]=PS3_REPORT_BUFFER[i];
  // https://github.com/felis/USB_Host_Shield_2.0/blob/master/PS3USB.cpp
  uint8_t cmd_buf[4];
  cmd_buf[0] = 0x42; // Special PS3 Controller enable commands
  cmd_buf[1] = 0x0c;
  cmd_buf[2] = 0x00;
  cmd_buf[3] = 0x00;
  this->set_report(0xf4, HID_REPORT_TYPE_FEATURE, cmd_buf, 4);
  delay(1);
};

void Pendant_PS3::set_leds(uint8_t leds)
{
  this->write_buffer[9] = leds;
  this->set_report(0x01, HID_REPORT_TYPE_OUTPUT, this->write_buffer, PS3_REPORT_BUFFER_SIZE);
}


void Pendant_PS3::process_buttons(const uint8_t *buttonbytebegin)
{
 uint32_t pressed_buttons_old = this->pressed_buttons;
 this->pressed_buttons = buttonbytebegin[0] | (buttonbytebegin[1]<<8) | (buttonbytebegin[2]<<16) | (buttonbytebegin[3]<<24);

 for(uint8_t i = 0 ; i <32 ; i++)
 {
  if( ((1<<i)&pressed_buttons_old) && !((1<<i)&this->pressed_buttons)   )
    this->on_key_release(i);
  if( !((1<<i)&pressed_buttons_old) && ((1<<i)&this->pressed_buttons)   )
    this->on_key_press(i);
 }
}

bool Pendant_PS3::is_button_pressed(uint8_t button)
{
  return (button<16 && (this->pressed_buttons&(1<<button)));
}

void Pendant_PS3::loop()
{
  /*
  static uint8_t leds = 0;
  static unsigned long last=millis();
  unsigned long now=millis();
  if( (now-last) > 2000)
  {
    last=now;
    set_leds((1<<leds));
    leds++;
    if(leds >4)
      leds=0;
  }
  */
}



void Pendant_PS3::report_received(uint8_t const *report, uint16_t len)
{
    if(len != 49 || report[0]!=1)
        return;
    // process keycodes
    this->process_buttons(report+2);
}

void Pendant_PS3::on_key_press(uint8_t keycode)
{
    Serial.print("Key Press: ");
    Serial.println(keycode, HEX);

    if(keycode==BUTTON_LEFT_TRIGGER || keycode==BUTTON_RIGHT_TRIGGER)
    {
      if(keycode==BUTTON_RIGHT_TRIGGER && this->step>0)
        this->step--;
      if(keycode==BUTTON_LEFT_TRIGGER && this->step<4)
        this->step++;
      Serial.println(this->step);
      if(this->step)
        this->set_leds(1<<(this->step));
      else
        this->set_leds(0);
    }
    
    float stepsize=0.0;
    if(this->step==1)
      stepsize = 0.01;
    else if(this->step==2)
      stepsize = 0.1;
    else if(this->step==3)
      stepsize = 1.0;
    else if(this->step==4)
      stepsize = 10.0;


    // Send ButtonCommands 
    if(keycode<=BUTTON_PS)
    {
        const char* cmd;
        cmd = PS3ButtonCommands[keycode];
        if(cmd[0])
        {
            String * cmdstr = new String(cmd);
            this->send_command(cmdstr);
        }
    }

    // Move Command
    if(this->step && ((keycode>=BUTTON_UP && keycode<=BUTTON_LEFT) || keycode==BUTTON_LEFT_SHOULDER || keycode==BUTTON_RIGHT_SHOULDER))
    {
      String * cmd = new String;
      if(keycode==BUTTON_LEFT || keycode==BUTTON_RIGHT)
        cmd->concat(PS3MoveCommands[0]);
      else if(keycode==BUTTON_DOWN || keycode==BUTTON_UP)
        cmd->concat(PS3MoveCommands[1]);
      else if(keycode==BUTTON_LEFT_SHOULDER || keycode==BUTTON_RIGHT_SHOULDER)
        cmd->concat(PS3MoveCommands[2]);

      if(keycode==BUTTON_RIGHT || keycode==BUTTON_UP || keycode==BUTTON_RIGHT_SHOULDER)
        cmd->concat(stepsize);
      else if(keycode==BUTTON_LEFT || keycode==BUTTON_DOWN || keycode==BUTTON_LEFT_SHOULDER)
        cmd->concat(-stepsize);

      this->send_command(cmd);
    }
}

void Pendant_PS3::on_key_release(uint8_t keycode)
{
    Serial.print("Key Release: ");
    Serial.println(keycode, HEX);
}