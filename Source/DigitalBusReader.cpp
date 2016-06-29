#include "DigitalBusReader.h"

// read a 4-byte data frame
void DigitalBusReader::readBusFrame(uint8_t* frame){
  // OWL Digital Bus Strix Protocol
  uint8_t id = frame[0]&0x0f;
  switch(frame[0]&0xf0){
  case 0:
    readMidiFrame(frame);
    break;
  case OWL_COMMAND_DISCOVER:
    handleDiscover(id, (frame[1] << 16) | (frame[2]<<8) | frame[3]);
    break;
  case OWL_COMMAND_ENUM:
    handleEnum(id, frame[1], frame[2] << 8, frame[3]);
    break;
  case OWL_COMMAND_IDENT:
    if(id != uid){
      handleIdent(id, frame[1], frame[2], frame[3]);
      if(id != nuid) // propagate
	sendFrame(frame);
    }
    break;
  case OWL_COMMAND_PARAMETER:
    if(id != uid){
      // it's not from us: process
      handleParameterChange(frame[1], (frame[2]<<8) | frame[3]);
      if(id != nuid) // propagate
	sendFrame(frame);
    }
    break;
  case OWL_COMMAND_DATA:
    // OSC, firmware or file data
    // 0x30, type, sizeH, sizeL
    // uint16_t size = (frame[2]<<8) | frame[3]);
    break;
  case OWL_COMMAND_SYNC:
    // 0xc0 until 0xff at end of frame
    // use ASCII SYN instead?
    break;
  default:
    debug << "bus reader rx unknown [" << frame[0] << "]\r\n";
    break;
  }
}

#if 0
  // read a 4-byte USB data frame
  // todo: dispatch directly to handleXYZ() methods
  MidiReaderStatus MidiReader::readFrame(unsigned char* frame){
    MidiReaderStatus status = INCOMPLETE_STATUS;
    switch(frame[0]){
    case USB_COMMAND_MISC:
    case USB_COMMAND_CABLE_EVENT:
      // ignore
      break;
    case USB_COMMAND_SINGLE_BYTE:
    case USB_COMMAND_SYSEX_EOX1:
      // one byte message
      status = read(frame[1]);
      break;
    case USB_COMMAND_2BYTE_SYSTEM_COMMON:
    case USB_COMMAND_SYSEX_EOX2:
    case USB_COMMAND_PROGRAM_CHANGE:
    case USB_COMMAND_CHANNEL_PRESSURE:
      read(frame[1]);
      status = read(frame[2]);
      break;
    case USB_COMMAND_NOTE_OFF:
    case USB_COMMAND_NOTE_ON:
    case USB_COMMAND_POLY_KEY_PRESSURE:
    case USB_COMMAND_CONTROL_CHANGE:
    case USB_COMMAND_PITCH_BEND_CHANGE:
      // three byte message
      read(frame[1]);
      read(frame[2]);
      status = read(frame[3]);
      break;
    default:
      switch(frame[0] & 0xf0){
      case OWL_COMMAND_DISCOVER:
	break;
      case OWL_COMMAND_PARAMETER:
	// parameter change
	handleParameterChange(frame[1], (frame[2]<<8) | frame[3]);
	status = READY_STATUS;
	break;
      case OWL_COMMAND_DATA:
	// OSC, firmware or file data
	// 0x30, type, sizeH, sizeL
	// uint16_t size = (frame[2]<<8) | frame[3]);
	break;
      case OWL_COMMAND_SYNC:
	// 0xc0 until 0xff at end of frame
	// use ASCII SYN instead?
	break;
      }
      // if((frame[0] & 0x0f) != uid && (frame[0] & 0x0f) != (uid+1)){
      // 	// we are not the originator
      // 	// forward message
      // }
      // ignore
    }
    return status;
  }
#endif
