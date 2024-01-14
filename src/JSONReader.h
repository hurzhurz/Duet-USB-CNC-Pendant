#ifndef JSONREADER_H
#define JSONREADER_H

#include <Arduino.h>

#define JSON_BUFFER_LEN 5000
class JSONReader
{
public:
  void write(uint8_t c);
  bool isReady();
  char * getBuffer();

private:
  char buffer[JSON_BUFFER_LEN];
  uint16_t length = 0;
  enum class State : uint8_t
  {
    waitingForStart = 0,
    waitingForNewLine,
    reading
  } state = State::waitingForStart;
  bool json_ready = false;
};

#endif