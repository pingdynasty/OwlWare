#ifndef _DigitalBusReader_h_
#define _DigitalBusReader_h_

#include "DigitalBusHandler.h"

class DigitalBusReader : public DigitalBusHandler {
 public:
  // read a 4-byte data frame
  void readFrame(uint8_t* frame);
private:
  void readSysex(uint8_t* data, int size);

};


#endif /* _DigitalBusReader_h_ */
