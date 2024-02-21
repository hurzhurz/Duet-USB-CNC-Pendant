#include "Pendant_WHB04B6.h"

// For Reference:
// https://github.com/LinuxCNC/linuxcnc/tree/master/src/hal/user_comps/xhc-WHB04B6



Pendant_WHB04B6::Pendant_WHB04B6(uint8_t dev_addr, uint8_t instance):
    USBHIDPendant(dev_addr, instance),
    axis_coordinates{0.0,0.0,0.0,0.0,0.0,0.0},
    display_report_data{ 0x06, 0xfe, 0xfd, SEED, 0x81, 0x00, 0x00, 0x00,
                          0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                          0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    jog(0),
    selected_axis(0),
    display_axis_offset(0),
    selected_feed(0)

{
    this->send_display_report();
    // DEMO DATA
    /*
    axis_coordinates[0]=-965.2345678;
    axis_coordinates[1]=1155.841333;
    axis_coordinates[2]=-44.8365;
    axis_coordinates[3]=34.224378;
    axis_coordinates[4]=-3335.1241333;
    axis_coordinates[5]=5564.1334465;
    */
};
Pendant_WHB04B6::~Pendant_WHB04B6()
{
    this->stop_continuous();
}
void Pendant_WHB04B6::report_received(uint8_t const *report, uint16_t len)
{
    if(len != 8 || report[0] != 0x04)
        return;
    uint8_t const &random = report[1];
    uint8_t const &keycode1 = report[2];
    uint8_t const &keycode2 = report[3];
    uint8_t const &feed = report[4];
    uint8_t const &axis = report[5];
    int8_t const &jogdelta = report[6];
    uint8_t const &checksum = report[7];

    uint8_t checksum_test = (keycode1)?(random - (keycode1 ^ (~SEED & random))):(random & SEED);
    if(checksum_test != checksum)
        return;

    bool display_update_needed = (keycode1)?true:false;

    this->jog += jogdelta;

    
    if(this->selected_feed != feed)
    {
        this->selected_feed = feed;
        this->jog = 0;
        this->stop_continuous();
    }

    if(this->selected_axis != axis)
    {
        this->selected_axis = axis;
        this->display_axis_offset = (axis>AXISSELCTOR_Z)?3:0;
        this->jog = 0;
        display_update_needed = true;
        this->stop_continuous();
    }

    this->process_keycodes(&keycode1, 2);

    
    if(display_update_needed)
    {
        this->send_display_report();
    }
}

void Pendant_WHB04B6::double_to_report_bytes(double val, uint8_t idx_intval_lower, uint8_t idx_intval_upper, uint8_t idx_frac_lower, uint8_t idx_frac_upper)
{
  uint16_t intval;
  uint16_t fraction;
  this->display_report_data[R_IDX(idx_frac_upper)] = 0;
  if(val<0.0)
  {
    val*=-1;
    this->display_report_data[R_IDX(idx_frac_upper)] = (1<<7);
  }
  intval = (uint16_t)val;
  fraction = (val-(float)intval)*10000;
  this->display_report_data[R_IDX(idx_intval_lower)] = intval&0xff;
  this->display_report_data[R_IDX(idx_intval_upper)] = ((intval&0xff00)>>8);
  this->display_report_data[R_IDX(idx_frac_lower)] = fraction&0xff;
  this->display_report_data[R_IDX(idx_frac_upper)] |= ((fraction&0x7f00)>>8);
}
void Pendant_WHB04B6::uint16_to_report_bytes(uint16_t val, uint8_t idx_lower, uint8_t idx_upper)
{
  this->display_report_data[R_IDX(idx_lower)] = val&0xff;
  this->display_report_data[R_IDX(idx_upper)] = ((val&0xff00)>>8);
}

void Pendant_WHB04B6::send_display_report()
{
    if(this->report_packet_next>0)
        return; // sending already in progress

    this->last_display_report = millis();

    // update axis coordinates in display report data
    this->double_to_report_bytes(axis_coordinates[0+this->display_axis_offset],4,5,6,7);
    this->double_to_report_bytes(axis_coordinates[1+this->display_axis_offset],8,9,10,11);
    this->double_to_report_bytes(axis_coordinates[2+this->display_axis_offset],12,13,14,15);

    // update mode indicator in display report data (mode not used yet!)
    uint8_t mode_bits = 0;
    if(this->mode == Mode::Step) mode_bits=0x01;
    this->display_report_data[R_IDX(3)] = (this->display_report_data[R_IDX(3)]&(~0x3)) | mode_bits;

    // send first packet of display report data to device
    this->set_report();
}

void Pendant_WHB04B6::set_report()
{
    if(this->report_packet_next>=REPORT_PACKET_COUNT)
    {
        // all packets sent
        this->report_packet_next = 0;
        return;
    }
    this->USBHIDPendant::set_report(0x06, HID_REPORT_TYPE_FEATURE, &(this->display_report_data[this->report_packet_next++ * 8]), 8);
}

void Pendant_WHB04B6::set_report_complete(uint8_t report_id, uint8_t report_type, uint16_t len)
{
    if(report_id!=0x06) // wrong report id?
        return;
    if(len!=8) // send failed ?
        this->report_packet_next = 0;
    else
        this->set_report(); // send next packet
}

void Pendant_WHB04B6::loop()
{ 
  static unsigned long last_cmd_check = millis();
  unsigned long now = millis();
  if( (now-last_cmd_check) > CMD_STEP_INTERVAL)
  {
    last_cmd_check = now;
    uint8_t feed = FEEDSELECTOR_TO_LINEAR(this->selected_feed);
    if(this->mode == Mode::Step && this->jog != 0 && this->selected_axis>=AXISSELCTOR_X && this->selected_axis<=AXISSELCTOR_C && feed && feed<=FEEDSELECTOR_STEP_STEPS)
    {
        float step_size = WHB04B6StepSizes[feed-1];
        uint8_t axis = this->selected_axis-AXISSELCTOR_X;
        String * cmd = new String(WHB04B6MoveCommands[axis]);
        cmd->concat(this->jog*step_size);
        this->send_command(cmd);
        this->jog = 0;
    }
  }
  if( this->mode == Mode::Continuous)
  {
    if((now-last_continuous_check) > CMD_CONTINUOUS_CHECK_INTERVAL)
    {
        this->handle_continuous_check();
    }
    if((now-last_continuous_update) > CMD_CONTINUOUS_UPDATE_INTERVAL)
    {
        this->handle_continuous_update();
    }
  }

  if( (now-this->last_display_report) > REPORT_INTERVAL)
  {
    this->send_display_report();
  }
}

void Pendant_WHB04B6::on_key_press(uint8_t keycode)
{
    Serial.print("Key Press: ");
    Serial.println(keycode, HEX);
    // mode
    if(keycode==KEYCODE_STEP)
    {
        this->jog = 0;
        this->stop_continuous();
        this->mode = Mode::Step;
    }
    if(keycode==KEYCODE_CONTINUOUS)
    {
        this->jog = 0;
        this->mode = Mode::Continuous;
        this->last_continuous_check = millis();
    }
    // Send ButtonCommands 
    if(keycode<=KEYCODE_M10)
    {
        const char* cmd;
        if(this->is_key_pressed(KEYCODE_FN))
            cmd = WHB04B6ButtonCommandsFN[keycode-1];
        else
            cmd = WHB04B6ButtonCommands[keycode-1];
        if(cmd[0])
        {
            String * cmdstr = new String(cmd);
            this->send_command(cmdstr);
        }
    }
}
void Pendant_WHB04B6::on_key_release(uint8_t keycode)
{
    Serial.print("Key Release: ");
    Serial.println(keycode, HEX);
}

void Pendant_WHB04B6::duetstatus_received(DuetStatus * duetstatus)
{
    for(uint8_t i=0;i<6;i++)
        this->axis_coordinates[i] = duetstatus->axis_userPosition[i];

    this->uint16_to_report_bytes(duetstatus->spindle_speed, 18,19);
    this->uint16_to_report_bytes((uint16_t)(duetstatus->speedFactor*100), 16,17);

    this->send_display_report();
}

void Pendant_WHB04B6::handle_continuous_check()
{
    this->last_continuous_check = millis();
    if(this->jog == 0)
    {
        this->stop_continuous();
    }
    else
    {
        bool direction = this->jog > 0;
        if(this->continuous_axis == 0 || this->continuous_direction!=direction)
        {
            this->continuous_axis = this->selected_axis-AXISSELCTOR_X+1;
            this->continuous_direction = direction;
            this->handle_continuous_update();
        }
    }
    this->jog = 0;
}
void Pendant_WHB04B6::handle_continuous_update()
{
    this->last_continuous_update = millis();
    uint8_t feed = FEEDSELECTOR_TO_LINEAR(this->selected_feed);
    if(this->continuous_axis && this->continuous_axis<=WHB04B6AxisCount && feed && feed<=FEEDSELECTOR_CONT_STEPS)
    {
        char cmd[100];
        sprintf(cmd, WHB04B6ContinuousRunCommand, WHB04B6AxisLetters[this->continuous_axis-1], (uint16_t)(WHB04B6ContinuousFeeds[this->continuous_axis-1]*WHB04B6ContinuousMultipliers[feed-1]) , this->continuous_direction?1:0 );
        String * cmdstr = new String(cmd);
        this->send_command(cmdstr);
    }
}
void Pendant_WHB04B6::stop_continuous()
{
  if(this->continuous_axis)
  {
    this->send_command(new String(WHB04B6ContinuousStopCommand));
    this->continuous_axis = 0;
  }
}