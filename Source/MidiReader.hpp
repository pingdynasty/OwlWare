#ifndef _MIDIREADER_H_
#define _MIDIREADER_H_

#include <inttypes.h>
#include "MidiStatus.h"

enum MidiReaderStatus {
  READY_STATUS, INCOMPLETE_STATUS, ERROR_STATUS
};

class MidiReader {
private:
  uint8_t* message;
  MidiReaderStatus status; // state, should be status: READY / INCOMPLETE / ERROR
  unsigned char runningStatus;
  int size;
  int pos;
public:
  MidiReader(uint8_t* buffer, uint16_t sz) : 
  message(buffer), 
    status(READY_STATUS), 
    runningStatus(0), 
    size(sz), 
    pos(0) {
  }

  ~MidiReader(){
  }

  virtual void handleSystemCommon(uint8_t){}
  virtual void handleProgramChange(uint8_t, uint8_t){}
  virtual void handleChannelPressure(uint8_t, uint8_t){}
  virtual void handleControlChange(uint8_t, uint8_t, uint8_t){}
  virtual void handleNoteOff(uint8_t, uint8_t, uint8_t){}
  virtual void handleNoteOn(uint8_t, uint8_t, uint8_t){}
  virtual void handlePitchBend(uint8_t, uint8_t, uint8_t){}
  virtual void handleSysEx(uint8_t* data, uint16_t size){}

  void clear(){
    runningStatus = message[0];
    pos = 0;
    status = READY_STATUS;
  }

  unsigned char* getMessage(int& length){
    length = pos;
    return message;
  }

  MidiReaderStatus read(unsigned char data){
    if(status == READY_STATUS){
      clear(); // discard previous message
    }else if(pos > size){
      status = ERROR_STATUS;
      // todo: throw exception
      return status;
    }
    message[pos++] = data;
    switch(message[0] & MIDI_STATUS_MASK){
      // two byte messages
    case PROGRAM_CHANGE:
      if(pos == 2){
	status = READY_STATUS;
	handleProgramChange(message[0], message[1]);
      }else{
	status = INCOMPLETE_STATUS;
      }
      break;
    case CHANNEL_PRESSURE:
      if(pos == 2){
	status = READY_STATUS;
	handleChannelPressure(message[0], message[1]);
      }else{
	status = INCOMPLETE_STATUS;
      }
      break;
      // three byte messages
    case NOTE_OFF:
      if(pos == 3){
	status = READY_STATUS;
	handleNoteOff(message[0], message[1], message[2]);
      }else{
	status = INCOMPLETE_STATUS;
      }
      break;
    case NOTE_ON:
      if(pos == 3){
	status = READY_STATUS;
	handleNoteOn(message[0], message[1], message[2]);
      }else{
	status = INCOMPLETE_STATUS;
      }
      break;
    case POLY_KEY_PRESSURE:
      if(pos == 3){
	status = READY_STATUS;
	handlePitchBend(message[0], message[1], message[2]);
      }else{
	status = INCOMPLETE_STATUS;
      }
      break;
    case CONTROL_CHANGE:
      if(pos == 3){
	status = READY_STATUS;
	handleControlChange(message[0], message[1], message[2]);
      }else{
	status = INCOMPLETE_STATUS;
      }
      break;
    case PITCH_BEND_CHANGE:
      if(pos == 3){
	status = READY_STATUS;
	handlePitchBend(message[0], message[1], message[2]);
      }else{
	status = INCOMPLETE_STATUS;
      }
      break;
    case SYSTEM_COMMON:
      switch(message[0]){
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
	handleSystemCommon(message[0]);
	break;
      case SYSEX:
	if(data == SYSEX_EOX){
	  status = READY_STATUS;
	  handleSysEx(message+1, pos-2);
	}else if(data >= STATUS_BYTE && pos > 1){
	  // SysEx message terminated by a status byte different from SYSEX_EOX
	  message[pos-1] = SYSEX_EOX;
	  status = READY_STATUS;
	  handleSysEx(message+1, pos-2);
	  message[0] = data; // save status byte for next message - will be saved as running status
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
	message[pos++] = data;
	message[0] = runningStatus;
      }else{
	status = ERROR_STATUS;
      }
    }
    return status;
  }

};


#endif /* _MIDIREADER_H_ */
