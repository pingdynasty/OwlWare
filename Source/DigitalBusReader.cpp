#include "DigitalBusReader.h"
#include "bus.h"
#include <string.h>
#include "device.h"

void DigitalBusReader::appendFrame(uint8_t* frame){

}

// read a 4-byte data frame
bool DigitalBusReader::readBusFrame(uint8_t* frame){
  // OWL Digital Bus Protocol
  uint8_t id = frame[0]&0x0f;
  switch(frame[0]&0xf0){
  case 0:
    if(!readMidiFrame(frame))
      return rxError("Invalid MIDI message");
#ifdef DIGITAL_BUS_PROPAGATE_MIDI
    sendFrame(frame); // warning: circular propagation!
#endif
    break;
  case OWL_COMMAND_DISCOVER:
    handleDiscover(id, (frame[1] << 16) | (frame[2]<<8) | frame[3]);
    break;
  // case OWL_COMMAND_ENUM:
  //   if(peers == 0)
  //     return rxError("Out of sequence enum message");
  //   handleEnum(id, frame[1], frame[2], frame[3]);
  //   break;
  // case OWL_COMMAND_IDENT:
  //   if(nuid == NO_UID)
  //     return rxError("Out of sequence ident message");
  //   if(id != uid){
  //     handleIdent(id, frame[1], frame[2], frame[3]);
  //     if(id != nuid) // propagate
  // 	sendFrame(frame);
  //   }
  //   break;
  case OWL_COMMAND_PARAMETER:
    if(id > peers)
      return rxError("Invalid bus message");
    handleParameterChange(frame[1], (frame[2]<<8) | frame[3]);
#ifdef DIGITAL_BUS_OUTPUT
    if(id > 1)
      sendFrame(frame[0]-1, frame[1], frame[2], frame[3]);
#endif
    break;
//   case OWL_COMMAND_BUTTON:
//     if(nuid == NO_UID)
//       return rxError("Out of sequence button message");
//     if(id != uid){
//       handleButtonChange(frame[1], (frame[2]<<8) | frame[3]);
// #ifdef DIGITAL_BUS_OUTPUT
//       if(id != nuid) // propagate
// 	sendFrame(frame);
// #endif
//     }
//     break;
  case OWL_COMMAND_COMMAND:
    // if(nuid == NO_UID)
    //   return rxError("Out of sequence command message");
    // if(id != uid){
    if(id > peers)
      return rxError("Invalid bus message");
    handleCommand(frame[1], (frame[2]<<8) | frame[3]);
#ifdef DIGITAL_BUS_OUTPUT
    if(id > 1)
      sendFrame(frame[0]-1, frame[1], frame[2], frame[3]);
#endif
// #ifdef DIGITAL_BUS_OUTPUT
//       if(id != nuid) // propagate
// 	sendFrame(frame);
// #endif
// }
    break;
  case OWL_COMMAND_MESSAGE:
    // if(nuid == NO_UID)
    //   return rxError("Out of sequence message");
    // if(id != uid){
    // if(appendFrame(frame));
    if(id > peers)
      return rxError("Invalid bus message");
    if(txuid == NO_UID)
      txuid = id;
    if(txuid == id){
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
#ifdef DIGITAL_BUS_OUTPUT
    if(id > 1)
      sendFrame(frame[0]-1, frame[1], frame[2], frame[3]);
#endif
//       }
// #ifdef DIGITAL_BUS_OUTPUT
//       if(id != nuid) // propagate
// 	sendFrame(frame);
// #endif
//     }
    break;
  case OWL_COMMAND_DATA:
    // if(nuid == NO_UID)
    //   return rxError("Out of sequence data message");
    // if(id != uid){
    if(id > peers)
      return rxError("Invalid bus message");
    if(txuid == NO_UID)
      txuid = id;
    if(txuid == id){
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
      // #ifdef DIGITAL_BUS_OUTPUT
      //       if(id != nuid) // propagate
      // 	sendFrame(frame);
      // #endif
      //     }
#ifdef DIGITAL_BUS_OUTPUT
    if(id > 1)
      sendFrame(frame[0]-1, frame[1], frame[2], frame[3]);
#endif
    break;
  case OWL_COMMAND_RESET:
    // if(id == 0){
    //   if(nuid != NO_UID) // propagate
    // 	sendFrame(frame);
    // }
    // #ifdef DIGITAL_BUS_OUTPUT
    //     if(id > 1)
    //       sendFrame(frame[0]-1, frame[1], frame[2], frame[3]);
    // #endif
    reset();
    break;
  default:
    return rxError("Invalid message");
    break;
  }
  return true;
}

void DigitalBusReader::reset(){
  MidiReader::reset();
  // uid = 0;
  // nuid = NO_UID;
  // token = NO_TOKEN;
  peers = 0;
  parameterOffset = 0;
  status = DigitalBusHandler::IDLE;
  txuid = NO_UID;
  datalen = 0;
}
