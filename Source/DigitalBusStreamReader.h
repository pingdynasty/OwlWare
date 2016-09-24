#ifndef _DigitalBusStreamReader_h_
#define _DigitalBusStreamReader_h_

#include "DigitalBusReader.h"
#include "SerialBuffer.hpp"

#define DIGITAL_BUS_RX_BUFFER_SIZE 256

class DigitalBusStreamReader : public DigitalBusReader {
public:
  DigitalBusStreamReader();
  /**
   * Read some data. The buffer pointed to by @param data must be at least 4 bytes long.
   */
  void read(uint8_t* data, uint16_t len){
    rxbuf.push(data, len);
  }
  void read(uint8_t data){
    rxbuf.push(data);
  }
  void process();
  void reset();
private:
  SerialBuffer<DIGITAL_BUS_RX_BUFFER_SIZE> rxbuf;
};

#endif /* _DigitalBusStreamReader_h_ */
