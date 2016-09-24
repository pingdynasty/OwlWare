#include "MidiReader.h"
#include "bus.h"
#include "message.h"

bool MidiReader::readMidiFrame(uint8_t* frame){
  // apparently no running status in USB MIDI frames
  switch(frame[0]){
  case USB_COMMAND_MISC:
  case USB_COMMAND_CABLE_EVENT:
    // ignore
    return false;
    break;
  case USB_COMMAND_SINGLE_BYTE:
    if((frame[1]&0xf0) != SYSTEM_COMMON)
      return false;
    handleSystemRealTime(frame[1]);
    break;
  case USB_COMMAND_2BYTE_SYSTEM_COMMON:
    if((frame[1]&0xf0) != SYSTEM_COMMON)
      return false;
    handleSystemCommon(frame[1], frame[2]);
    break;
  case USB_COMMAND_3BYTE_SYSTEM_COMMON:
    if((frame[1]&0xf0) != SYSTEM_COMMON)
      return false;
    handleSystemCommon(frame[1], frame[2], frame[3]);
    break;
  case USB_COMMAND_SYSEX_EOX1:
    if(pos < 3 || buffer[0] != SYSEX || frame[1] != SYSEX_EOX){
      return false;
      // bus_rx_error("Invalid SysEx");
    }else if(pos >= size){
      return false;
      // bus_rx_error("SysEx buffer overflow");
    }else{
      buffer[pos++] = frame[1];
      handleSysEx(buffer, pos);
    }
    pos = 0;
    break;
  case USB_COMMAND_SYSEX_EOX2:
    if(pos < 3 || buffer[0] != SYSEX || frame[2] != SYSEX_EOX){
      return false;
      // bus_rx_error("Invalid SysEx");
    }else if(pos+2 > size){
      return false;
      // bus_rx_error("SysEx buffer overflow");
    }else{
      buffer[pos++] = frame[1];
      buffer[pos++] = frame[2];
      handleSysEx(buffer, pos);
    }
    pos = 0;
    break;
  case USB_COMMAND_SYSEX_EOX3:
    if(pos < 3 || buffer[0] != SYSEX || frame[3] != SYSEX_EOX){
      return false;
      // bus_rx_error("Invalid SysEx");
    }else if(pos+3 > size){
      return false;
      // bus_rx_error("SysEx buffer overflow");
    }else{
      buffer[pos++] = frame[1];
      buffer[pos++] = frame[2];
      buffer[pos++] = frame[3];
      handleSysEx(buffer, pos);
    }
    pos = 0;
    break;
  case USB_COMMAND_SYSEX:
    if(pos+3 > size){
      return false;
      // bus_rx_error("SysEx buffer overflow");
    }else{
      buffer[pos++] = frame[1];
      buffer[pos++] = frame[2];
      buffer[pos++] = frame[3];
    }
    break;
  case USB_COMMAND_PROGRAM_CHANGE:
    if((frame[1]&0xf0) != PROGRAM_CHANGE)
      return false;
    handleProgramChange(frame[1], frame[2]);
    break;
  case USB_COMMAND_CHANNEL_PRESSURE:
    if((frame[1]&0xf0) != CHANNEL_PRESSURE)
      return false;
    handleChannelPressure(frame[1], frame[2]);
    break;
  case USB_COMMAND_NOTE_OFF:
    if((frame[1]&0xf0) != NOTE_OFF)
      return false;
    handleNoteOff(frame[1], frame[2], frame[3]);
    break;
  case USB_COMMAND_NOTE_ON:
    if((frame[1]&0xf0) != NOTE_ON)
      return false;
    if(frame[3] == 0)
      handleNoteOff(frame[1], frame[2], frame[3]);
    else
      handleNoteOn(frame[1], frame[2], frame[3]);
    break;
  case USB_COMMAND_POLY_KEY_PRESSURE:
    if((frame[1]&0xf0) != POLY_KEY_PRESSURE)
      return false;
    handlePolyKeyPressure(frame[1], frame[2], frame[3]);
    break;
  case USB_COMMAND_CONTROL_CHANGE:
    if((frame[1]&0xf0) != CONTROL_CHANGE)
      return false;
    handleControlChange(frame[1], frame[2], frame[3]);
    break;
  case USB_COMMAND_PITCH_BEND_CHANGE:
    if((frame[1]&0xf0) != PITCH_BEND_CHANGE)
      return false;
    handlePitchBend(frame[1], frame[2] | (frame[3]<<7));
    break;
  default:
    return false;
    // bus_rx_error("Invalid USB MIDI message");
    break;
  }
  return true;
}

void MidiReader::reset(){
  pos = 0;
}
