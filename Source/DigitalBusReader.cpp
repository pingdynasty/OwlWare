#include "DigitalBusReader.h"
#include "bus.h"
#include <string.h>

// read a 4-byte data frame
bool DigitalBusReader::readBusFrame(uint8_t* frame){
  // OWL Digital Bus Protocol
  uint8_t seq = frame[0]&0x0f;
  switch(frame[0]&0xf0){
  case 0:
    if(!readMidiFrame(frame))
      return rxError("Invalid MIDI message");
    if(DIGITAL_BUS_PROPAGATE_MIDI)
      sendFrame(frame); // warning: circular propagation!
    break;
  case OWL_COMMAND_DISCOVER:
    handleDiscover(seq, (frame[1] << 16) | (frame[2]<<8) | frame[3]);
    break;
  case OWL_COMMAND_PARAMETER:
    if(seq > peers)
      return rxError("Invalid bus parameter");
    handleParameterChange(frame[1], (frame[2]<<8) | frame[3]);
    if(DIGITAL_BUS_ENABLE_BUS && seq > 1)
      sendFrame(frame[0]-1, frame[1], frame[2], frame[3]);
    break;
  case OWL_COMMAND_COMMAND:
    if(seq > peers)
      return rxError("Invalid bus command");
    handleCommand(frame[1], (frame[2]<<8) | frame[3]);
    if(DIGITAL_BUS_ENABLE_BUS && seq > 1)
      sendFrame(frame[0]-1, frame[1], frame[2], frame[3]);
    break;
  case OWL_COMMAND_MESSAGE:
    if(seq > peers)
      return rxError("Invalid bus message");
    if(txuid == NO_UID)
      txuid = seq;
    if(txuid == seq){
      // ignore if we are not exclusively listening to long messages from this uid
      if(pos+3 < size){
        strncpy((char*)buffer+pos, (char*)frame+1, 3);
	pos += 3;
      }else{
	// buffer overflow
      }
      if(frame[3] == '\0'){
	pos = 0;
	txuid = NO_UID;
	handleMessage((const char*)buffer);
      }
    }
    if(DIGITAL_BUS_ENABLE_BUS && seq > 1)
      sendFrame(frame[0]-1, frame[1], frame[2], frame[3]);
    break;
  case OWL_COMMAND_DATA:
    if(seq > peers)
      return rxError("Invalid bus data");
    if(txuid == NO_UID)
      txuid = seq;
    if(txuid == seq){
      if(datalen == 0){
	datalen = (frame[1]<<16) | (frame[2]<<8) | frame[3];
	pos = 0;
      }else if(pos+3 < size){
	buffer[pos++] = frame[1];
	buffer[pos++] = frame[2];
	buffer[pos++] = frame[3];
      }else{
	return rxError("Data buffer overflow");
      }
      if(pos >= datalen){
	handleData(buffer, datalen);
	txuid = NO_UID;
	datalen = 0;
	pos = 0;
      }
    }
    if(DIGITAL_BUS_ENABLE_BUS && seq > 1)
      sendFrame(frame[0]-1, frame[1], frame[2], frame[3]);
    break;
  case OWL_COMMAND_RESET:
    if(DIGITAL_BUS_ENABLE_BUS && peers > 1){
      reset();
      sendReset();
    }else{
      reset();
    }
    break;
  default:
    return rxError("Invalid bus message");
    break;
  }
  return true;
}

void DigitalBusReader::reset(){
  MidiReader::reset();
  peers = 0;
  parameterOffset = 0;
  status = DigitalBusHandler::IDLE;
  txuid = NO_UID;
  datalen = 0;
}
