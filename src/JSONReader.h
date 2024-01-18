#ifndef JSONREADER_H
#define JSONREADER_H

#include <Arduino.h>

#define JSONREADER_DEFAULT_BUFFER_LEN 5000
class JSONReader
{
public:
  JSONReader(uint16_t buffer_size = JSONREADER_DEFAULT_BUFFER_LEN, const char * filter_select = 0, const char * filter_discard = 0);
  ~JSONReader();
  void write(uint8_t c);
  bool isReady();
  char * getBuffer();

private:
  char * buffer;
  char * filter_select = 0;
  char * filter_discard = 0;
  bool filter_select_done, filter_discard_done;
  uint16_t filter_select_len = 0;
  uint16_t filter_discard_len = 0;
  uint16_t buffer_size;
  uint16_t length = 0;
  enum class State : uint8_t
  {
    waitingForStart = 0,
    waitingForNewLine,
    filtering,
    reading
  } state = State::waitingForStart;
  bool json_ready = false;
};

#endif