#include "Pendant_Numpad.h"

#define CMD_CONTINUOUS_INTERVAL 500


void Pendant_Numpad::stop_continuous()
{
  if(this->continuous_axis)
  {
    this->send_command(new String(NumpadContinuousStopCommand));
    this->continuous_axis = 0;
  }
}
void Pendant_Numpad::handle_continuous()
{
    if(this->continuous_axis && this->continuous_axis<=NumpadAxisCount && this->speed_step)
    {
        char cmd[100];
        sprintf(cmd, NumpadContinuousRunCommand, NumpadAxisLetters[this->continuous_axis-1], (uint16_t)(NumpadContinuousFeeds[this->continuous_axis-1]*NumpadContinuousMultipliers[this->speed_step-1]) , this->continuous_direction?1:0 );
        String * cmdstr = new String(cmd);
        this->send_command(cmdstr);
    }
}

void Pendant_Numpad::loop()
{
  /*
  static unsigned long last=millis();
  unsigned long now=millis();
  if( (now-last) > 2000)
  {
    last=now;
    this->set_report(0, HID_REPORT_TYPE_OUTPUT, &leds, sizeof(leds));
  }
  */
  static unsigned long last_cmd_check = millis();
  unsigned long now = millis();
  if( (now-last_cmd_check) > CMD_CONTINUOUS_INTERVAL)
  {
    last_cmd_check = now;
    this->handle_continuous();
  }
}

void Pendant_Numpad::toogle_numlockled()
{
    this->set_numlockled(leds==0);
}
void Pendant_Numpad::set_numlockled(bool state)
{
    if(state)
      leds=KEYBOARD_LED_NUMLOCK;
    else
      leds=0;
    this->set_report(0, HID_REPORT_TYPE_OUTPUT, &leds, sizeof(leds));
}

void Pendant_Numpad::report_received(uint8_t const *report, uint16_t len)
{
    // check if valid length
    if(len != 8)
        return;
    // process keycodes
    this->process_keycodes(report+2, 6);
}

void Pendant_Numpad::on_key_press(uint8_t keycode)
{
    Serial.print("Key Press: ");
    Serial.println(keycode, HEX);
    char keycodestr[4];
    itoa(keycode,keycodestr,10);
    JsonVariant keyconfig = (*config)["keys"][keycodestr];
    if(!keyconfig.isNull())
    {
      const char* cmd = 0;

      // Send ButtonCommands
      uint8_t altkey = keyconfig["alt_key"];
      if(altkey && this->is_key_pressed(altkey))
        cmd = keyconfig["alt_command"];
      else
        cmd = keyconfig["command"];
      if(cmd && cmd[0])
          this->send_command(new String(cmd));

      
    }
    // Change Distance per KeyPress
    if(keycode == KEYCODE_NUMLOCK)
    {
      this->speed_step=0; // OFF
      this->stop_continuous();
    }
    else if(keycode == KEYCODE_DIV)
      this->speed_step=1;
    else if(keycode == KEYCODE_MULT)
      this->speed_step=2;
    else if(keycode == KEYCODE_BACKSPACE)
      this->speed_step=3;

    if(keycode == KEYCODE_DOT_DEL) // set Continuouss Mode
    {
      this->continuous_mode=true;
    }
    else if(keycode == KEYCODE_0_INS) // set Step Mode
    {
      this->continuous_mode=false;
      this->stop_continuous();
    } 
    
    // Move Command / Step Mode
    if(!this->continuous_mode && this->speed_step && !keyconfig.isNull() && !keyconfig["move_axis"].isNull())
    {
      uint8_t axis = keyconfig["move_axis"];
      if(axis<NumpadAxisCount)
      {
        String * cmd = new String(NumpadMoveCommands[axis]);
        float step = NumpadStepSizes[this->speed_step-1];
        bool direction = keyconfig["move_direction"] | true;
        if(!direction)
          step = -step;
        cmd->concat(step);
        this->send_command(cmd);
      }
    }

    if(this->continuous_mode && !this->continuous_axis) // start continuous move
    {
      if(!keyconfig.isNull() && !keyconfig["move_axis"].isNull())
      {
          uint8_t axis = keyconfig["move_axis"];
          if(axis<NumpadAxisCount)
          {
            this->continuous_axis=axis+1;
            this->continuous_direction= keyconfig["move_direction"] | true;
            this->continuous_key=keycode;
          }
      }
      if(this->continuous_axis)
        this->handle_continuous();
    }

    // Toogle NumLock LED to indicate keypress was processed
    this->toogle_numlockled();
}

void Pendant_Numpad::on_key_release(uint8_t keycode)
{
    Serial.print("Key Release: ");
    Serial.println(keycode, HEX);
    if(this->continuous_axis && keycode==this->continuous_key)
    {
      stop_continuous();
    }
}