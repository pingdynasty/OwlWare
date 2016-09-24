#include "DigitalBusStreamReader.h"
#include "owlcontrol.h"

DigitalBusStreamReader::DigitalBusStreamReader(){
}

void DigitalBusStreamReader::process(){
  while(rxbuf.available() >= 4){
    debugClear();
    uint8_t frame[4];
    rxbuf.pull(frame, 4);
    if(frame[0] == OWL_COMMAND_RESET){
      rxbuf.skipUntilLast(OWL_COMMAND_RESET);
    }else{
      if(readBusFrame(frame))
	debugSet();
    }
  }
}

void DigitalBusStreamReader::reset(){
  DigitalBusReader::reset();
  rxbuf.reset();
}
