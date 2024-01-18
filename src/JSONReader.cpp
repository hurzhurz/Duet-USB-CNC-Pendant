#include "JSONReader.h"

JSONReader::JSONReader(uint16_t buffer_size, const char * filter_select, const char * filter_discard)
{
    if(filter_select && *filter_select)
    {
        this->filter_select_len = strlen(filter_select);
        this->filter_select = new char[this->filter_select_len+1];
        strcpy(this->filter_select, filter_select);
    }
    if(filter_discard && *filter_discard)
    {
        this->filter_discard_len = strlen(filter_discard);
        this->filter_discard = new char[this->filter_discard_len+1];
        strcpy(this->filter_discard, filter_discard);
    }
    this->buffer_size = buffer_size;
    this->buffer = new char[buffer_size];
}
JSONReader::~JSONReader()
{
    delete[] this->buffer;
    delete[] this->filter_select;
    delete[] this->filter_discard;
}


void JSONReader::write(uint8_t c)
{   
    if(this->state==State::waitingForStart)
    {  
        if(c != '\n' && c != '\r')
        {
            if(c == '{' && !this->json_ready)
            {
                this->filter_select_done = this->filter_select==0;
                this->filter_discard_done = this->filter_discard==0;
                this->state = (this->filter_select_done&&this->filter_discard_done)?State::reading:State::filtering;
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

    if(this->state==State::reading || this->state==State::filtering)
    {
        if (c == '\n' || c == '\r')
        {
            if(this->buffer[this->length-1]=='}' && this->state!=State::filtering)
            {
                this->buffer[this->length]=0;
                this->json_ready = true;
            }
            this->state = State::waitingForStart;
        }
        else
        {
            if((this->length+1) < this->buffer_size)
            {
                this->buffer[this->length++] = c;
                if(this->state==State::filtering)
                {
                    if(!this->filter_select_done)
                    {
                        if(this->filter_select[this->length-1] != c)
                            this->state = State::waitingForNewLine;
                        else if(this->filter_select[this->length] == 0) // end of filter - still matching
                            this->filter_select_done = true;
                    }
                    if(!this->filter_discard_done)
                    {
                        if(this->filter_discard[this->length-1] != c)
                            this->filter_discard_done = true;
                        else if(this->filter_discard[this->length] == 0) // end of filter - still matching
                            this->state = State::waitingForNewLine;
                    }
                    if(this->state==State::filtering && this->filter_select_done && this->filter_discard_done)
                       this->state = State::reading;
                }
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
