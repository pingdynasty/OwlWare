#include "DigitalBusStreamReader.h"

DigitalBusStreamReader::DigitalBusStreamReader(){
}

// extern "C" void midi_tx_usb_buffer(uint8_t* buffer, uint32_t length);

void DigitalBusStreamReader::process(){
  while(rxbuf.available() >= 4){
    uint8_t frame[4];
    rxbuf.pull(frame, 4);
    if(frame[0] == OWL_COMMAND_RESET){
      rxbuf.skipUntilLast(OWL_COMMAND_RESET);
    }else{
      readBusFrame(frame);
      // if(readBusFrame(frame) && isMidiFrame(frame))
      // 	midi_tx_usb_buffer(frame, 4); // forward serial bus to USB MIDI
    }
  }
}

void DigitalBusStreamReader::reset(){
  DigitalBusReader::reset();
  rxbuf.reset();
}
