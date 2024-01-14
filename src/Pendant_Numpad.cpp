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

    // Send ButtonCommands
    if(keycode==KEYCODE_BACKSPACE || (keycode>=KEYCODE_NUMLOCK && keycode<=KEYCODE_DOT_DEL))
    {
        const char* cmd;
        if(keycode==KEYCODE_BACKSPACE)
            cmd = NumpadButtonCommands[0];
        else
            cmd = NumpadButtonCommands[keycode-KEYCODE_NUMLOCK+1];
        if(cmd[0])
        {
            String * cmdstr = new String(cmd);
            this->send_command(cmdstr);
        }
    }
    
    // Move Command / Step Mode
    if(!this->continuous_mode && this->speed_step && (keycode==KEYCODE_SUB || keycode==KEYCODE_ADD || keycode==KEYCODE_2_DOWN || keycode==KEYCODE_4_LEFT || keycode==KEYCODE_6_RIGHT || keycode==KEYCODE_8_UP))
    {
      String * cmd = new String;
      if(keycode==KEYCODE_4_LEFT || keycode==KEYCODE_6_RIGHT)
        cmd->concat(NumpadMoveCommands[0]);
      else if(keycode==KEYCODE_2_DOWN || keycode==KEYCODE_8_UP)
        cmd->concat(NumpadMoveCommands[1]);
      else if(keycode==KEYCODE_SUB || keycode==KEYCODE_ADD)
        cmd->concat(NumpadMoveCommands[2]);

      if(keycode==KEYCODE_6_RIGHT || keycode==KEYCODE_8_UP || keycode==KEYCODE_SUB)
        cmd->concat(NumpadStepSizes[this->speed_step-1]);
      else if(keycode==KEYCODE_4_LEFT || keycode==KEYCODE_2_DOWN || keycode==KEYCODE_ADD)
        cmd->concat(-NumpadStepSizes[this->speed_step-1]);

      this->send_command(cmd);
    }

    if(this->continuous_mode && !this->continuous_axis) // start continuous move
    {
      switch(keycode)
      {
        case KEYCODE_6_RIGHT:
          this->continuous_axis=1;
          this->continuous_direction=true;
          break;
        case KEYCODE_4_LEFT:
          this->continuous_axis=1;
          this->continuous_direction=false;
          break;
        case KEYCODE_8_UP:
          this->continuous_axis=2;
          this->continuous_direction=true;
          break;
        case KEYCODE_2_DOWN:
          this->continuous_axis=2;
          this->continuous_direction=false;
          break;
        case KEYCODE_SUB:
          this->continuous_axis=3;
          this->continuous_direction=true;
          break;
        case KEYCODE_ADD:
          this->continuous_axis=3;
          this->continuous_direction=false;
          break;
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
    if(this->continuous_axis)
    {
      if(
          (keycode==KEYCODE_6_RIGHT && this->continuous_axis==1 &&  this->continuous_direction) ||
          (keycode==KEYCODE_4_LEFT  && this->continuous_axis==1 && !this->continuous_direction) ||
          (keycode==KEYCODE_8_UP    && this->continuous_axis==2 &&  this->continuous_direction) ||
          (keycode==KEYCODE_2_DOWN  && this->continuous_axis==2 && !this->continuous_direction) ||
          (keycode==KEYCODE_SUB     && this->continuous_axis==3 &&  this->continuous_direction) ||
          (keycode==KEYCODE_ADD     && this->continuous_axis==3 && !this->continuous_direction)
        )
        stop_continuous();
    }
}