#include "DigitalBusReader.h"

// read a 4-byte data frame
void DigitalBusReader::readFrame(uint8_t* frame){
  switch(frame[0]){
  case USB_COMMAND_MISC:
  case USB_COMMAND_CABLE_EVENT:
    // ignore
    break;
  case USB_COMMAND_SINGLE_BYTE:
    handleSystemCommon(frame[1]);
    break;
  case USB_COMMAND_2BYTE_SYSTEM_COMMON:
    handleSystemCommon(frame[1], frame[2]);
    break;
  case USB_COMMAND_3BYTE_SYSTEM_COMMON:
    handleSystemCommon(frame[1], frame[2], frame[3]);
    break;
  case USB_COMMAND_SYSEX_EOX1:
    readSysex(&frame[1], 1);
    break;
  case USB_COMMAND_SYSEX_EOX2:
    readSysex(&frame[1], 2);
    break;
  case USB_COMMAND_SYSEX:
  case USB_COMMAND_SYSEX_EOX3:
    readSysex(&frame[1], 3);
    break;
  case USB_COMMAND_PROGRAM_CHANGE:
    handleProgramChange(frame[1], frame[2]);
    break;
  case USB_COMMAND_CHANNEL_PRESSURE:
    handleChannelPressure(frame[1], frame[2]);
    break;
  case USB_COMMAND_NOTE_OFF:
    handleNoteOff(frame[1], frame[2], frame[3]);
    break;
  case USB_COMMAND_NOTE_ON:
    if(frame[3] == 0)
      handleNoteOff(frame[1], frame[2], frame[3]);
    else
      handleNoteOn(frame[1], frame[2], frame[3]);
    break;
  case USB_COMMAND_POLY_KEY_PRESSURE:
    handlePolyKeyPressure(frame[1], frame[2], frame[3]);
    break;
  case USB_COMMAND_CONTROL_CHANGE:
    handleControlChange(frame[1], frame[2], frame[3]);
    break;
  case USB_COMMAND_PITCH_BEND_CHANGE:
    handlePitchBend(frame[1], frame[2] | (frame[3]<<7));
    break;
  default:
    // OWL Digital Bus Strix Protocol
    switch(frame[0] & 0xf0){
    case OWL_COMMAND_DISCOVER:
      handleDiscover(frame[0]&0x0f, (frame[1] << 16) | (frame[2]<<8) | frame[3]);
      break;
    case OWL_COMMAND_ENUM:
      handleEnum(frame[0]&0x0f, frame[1], frame[2] << 8, frame[3]);
      break;
    case OWL_COMMAND_IDENT:
      handleIdent(frame[0]&0x0f, frame[1], frame[2], frame[3]);
      break;
    case OWL_COMMAND_PARAMETER:
      if((frame[0]&0x0f) != uid){
	// it's not from us
	handleParameterChange(frame[1], (frame[2]<<8) | frame[3]);
	// propagate
	sendMessage(frame[0], frame[1], frame[2], frame[3]);
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
      break;
    }
    // if((frame[0] & 0x0f) != uid && (frame[0] & 0x0f) != (uid+1)){
    // 	// we are not the originator
    // 	// forward message
    // }
    // ignore
  }
}

void DigitalBusReader::readSysex(uint8_t* data, int size){
  for(int i=0; i<size; ++i){
    if(data[i] == SYSEX_EOX){
      status = READY_STATUS;
      handleSysEx(buffer+1, pos-2);
    }else if(data[i] >= STATUS_BYTE && pos > 1){
      // SysEx message terminated by a status byte different from SYSEX_EOX
      buffer[pos-1] = SYSEX_EOX;
      status = READY_STATUS;
      handleSysEx(buffer+1, pos-2);
      buffer[0] = data[i]; // save status byte for next message - will be saved as running status
    }else{
      if(pos < size){
	buffer[pos++] = data[i];
	status = INCOMPLETE_STATUS;
      }else{
	status = ERROR_STATUS;
      }
    }
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
