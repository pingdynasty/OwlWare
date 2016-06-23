#include "MidiReader.h"

MidiReaderStatus MidiReader::read(unsigned char data){
  if(status == READY_STATUS){
    clear(); // discard previous message
  }else if(pos > size){
    status = ERROR_STATUS;
    // todo: throw exception
    return status;
  }
  buffer[pos++] = data;
  switch(buffer[0] & MIDI_STATUS_MASK){
    // two byte messages
  case PROGRAM_CHANGE:
    if(pos == 2){
      status = READY_STATUS;
      handleProgramChange(buffer[0], buffer[1]);
    }else{
      status = INCOMPLETE_STATUS;
    }
    break;
  case CHANNEL_PRESSURE:
    if(pos == 2){
      status = READY_STATUS;
      handleChannelPressure(buffer[0], buffer[1]);
    }else{
      status = INCOMPLETE_STATUS;
    }
    break;
    // three byte messages
  case NOTE_OFF:
    if(pos == 3){
      status = READY_STATUS;
      handleNoteOff(buffer[0], buffer[1], buffer[2]);
    }else{
      status = INCOMPLETE_STATUS;
    }
    break;
  case NOTE_ON:
    if(pos == 3){
      status = READY_STATUS;
      if(buffer[2] == 0)
	handleNoteOff(buffer[0], buffer[1], buffer[2]);
      else
	handleNoteOn(buffer[0], buffer[1], buffer[2]);
    }else{
      status = INCOMPLETE_STATUS;
    }
    break;
  case POLY_KEY_PRESSURE:
    if(pos == 3){
      status = READY_STATUS;
      handlePolyKeyPressure(buffer[0], buffer[1], buffer[2]);
    }else{
      status = INCOMPLETE_STATUS;
    }
    break;
  case CONTROL_CHANGE:
    if(pos == 3){
      status = READY_STATUS;
      handleControlChange(buffer[0], buffer[1], buffer[2]);
    }else{
      status = INCOMPLETE_STATUS;
    }
    break;
  case PITCH_BEND_CHANGE:
    if(pos == 3){
      status = READY_STATUS;
      handlePitchBend(buffer[0], (buffer[2]<<7) | buffer[1]);
    }else{
      status = INCOMPLETE_STATUS;
    }
    break;
  case SYSTEM_COMMON:
    switch(buffer[0]){
    case TIME_CODE_QUARTER_FRAME:
    case RESERVED_F4:
    case RESERVED_F9:
    case TUNE_REQUEST:
    case TIMING_CLOCK:
    case START:
    case CONTINUE:
    case STOP:
    case RESERVED_FD:
    case ACTIVE_SENSING:
    case SYSTEM_RESET:
      // one byte messages
      status = READY_STATUS;
      handleSystemCommon(buffer[0]);
      break;
    case SYSEX:
      if(data == SYSEX_EOX){
	status = READY_STATUS;
	handleSysEx(buffer+1, pos-2);
      }else if(data >= STATUS_BYTE && pos > 1){
	// SysEx message terminated by a status byte different from SYSEX_EOX
	buffer[pos-1] = SYSEX_EOX;
	status = READY_STATUS;
	handleSysEx(buffer+1, pos-2);
	buffer[0] = data; // save status byte for next message - will be saved
      }else{
	status = INCOMPLETE_STATUS;
      }
      break;
    case SYSEX_EOX: // receiving SYSEX_EOX on its own is really an error
    default:
      status = ERROR_STATUS;
      break;
    }
    break;
  default:
    if(pos == 1 && data < STATUS_BYTE && runningStatus >= STATUS_BYTE){
      // MIDI running status: this message is missing the status byte, re-use previous status
      buffer[pos++] = data;
      buffer[0] = runningStatus;
    }else{
      status = ERROR_STATUS;
    }
  }
  return status;
}
