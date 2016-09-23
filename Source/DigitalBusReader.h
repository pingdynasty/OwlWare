#ifndef _DigitalBusReader_h_
#define _DigitalBusReader_h_

#include "DigitalBusHandler.h"

#define DIGITAL_BUS_PROPAGATE_MIDI
/* #define DIGITAL_BUS_OUTPUT */

class DigitalBusReader : public DigitalBusHandler {
public:
  // read a 4-byte data frame
  void readBusFrame(uint8_t* frame);
  void reset();
private:
  void appendFrame(uint8_t* frame);
  uint8_t txuid = NO_UID;
  uint32_t datalen = 0;
};

#endif /* _DigitalBusReader_h_ */
