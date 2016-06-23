#ifndef _DigitalBusReader_h_
#define _DigitalBusReader_h_

#include "DigitalBusHandler.h"

class DigitalBusReader : public DigitalBusHandler {
 public:
  // read a 4-byte data frame
  void readBusFrame(uint8_t* frame);
};


#endif /* _DigitalBusReader_h_ */
