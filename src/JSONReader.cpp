#include "JSONReader.h"

void JSONReader::write(uint8_t c)
{   
    if(this->state==State::waitingForStart)
    {  
        if(c != '\n' && c != '\r')
        {
            if(c == '{' && !this->json_ready)
            {
                this->state = State::reading;
                this->length = 0;
            }
            else
            {
                state = State::waitingForNewLine;
            }
        }
    }
    else if(this->state==State::waitingForNewLine)
    {
        if (c == '\n' || c == '\r')
            this->state = State::waitingForStart;
    }

    if(this->state==State::reading)
    {
        if (c == '\n' || c == '\r')
        {
            if(this->buffer[this->length-1]=='}')
            {
                this->buffer[this->length]=0;
                this->json_ready = true;
            }
            this->state = State::waitingForStart;
        }
        else
        {
            if((this->length+1)<JSON_BUFFER_LEN)
            {
                this->buffer[this->length++] = c;
            }
            else
            {
                // buffer overflow, restart
                Serial.println("overflow");
                state = State::waitingForNewLine;
            }
        }
    }

}

bool JSONReader::isReady()
{
    return this->json_ready;
}
char * JSONReader::getBuffer()
{
    this->json_ready = false;
    return this->buffer;
}
