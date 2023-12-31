#include "Pendant_Numpad.h"


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
      this->multiplier=0.0; // OFF
    else if(keycode == KEYCODE_DIV)
      this->multiplier=0.1;
    else if(keycode == KEYCODE_MULT)
      this->multiplier=1.0;
    else if(keycode == KEYCODE_BACKSPACE)
      this->multiplier=10.0;

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
    
    // Move Command
    if(this->multiplier>0.0 && (keycode==KEYCODE_SUB || keycode==KEYCODE_ADD || keycode==KEYCODE_2_DOWN || keycode==KEYCODE_4_LEFT || keycode==KEYCODE_6_RIGHT || keycode==KEYCODE_8_UP))
    {
      String * cmd = new String;
      if(keycode==KEYCODE_4_LEFT || keycode==KEYCODE_6_RIGHT)
        cmd->concat(NumpadMoveCommands[0]);
      else if(keycode==KEYCODE_2_DOWN || keycode==KEYCODE_8_UP)
        cmd->concat(NumpadMoveCommands[1]);
      else if(keycode==KEYCODE_SUB || keycode==KEYCODE_ADD)
        cmd->concat(NumpadMoveCommands[2]);

      if(keycode==KEYCODE_6_RIGHT || keycode==KEYCODE_8_UP || keycode==KEYCODE_SUB)
        cmd->concat(this->multiplier);
      else if(keycode==KEYCODE_4_LEFT || keycode==KEYCODE_2_DOWN || keycode==KEYCODE_ADD)
        cmd->concat(-this->multiplier);

      this->send_command(cmd);
    }
    // Toogle NumLock LED to indicate keypress was processed
    this->toogle_numlockled();
}

void Pendant_Numpad::on_key_release(uint8_t keycode)
{
    Serial.print("Key Release: ");
    Serial.println(keycode, HEX);
}