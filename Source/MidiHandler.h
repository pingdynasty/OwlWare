#ifndef _MidiHandler_h_
#define _MidiHandler_h_

#include <stdint.h>
#include "device.h"

class MidiHandler {
protected:
  int8_t channel;
public:
  MidiHandler();
  void handleProgramChange(uint8_t status, uint8_t pc);
  void handleControlChange(uint8_t status, uint8_t cc, uint8_t value);
  void handleNoteOff(uint8_t status, uint8_t note, uint8_t velocity);
  void handleNoteOn(uint8_t status, uint8_t note, uint8_t velocity);
  void handlePitchBend(uint8_t status, uint16_t value);
  void handleSysEx(uint8_t* data, uint16_t size);

  void handleSystemRealTime(uint8_t cmd){}
  void handleSystemCommon(uint8_t cmd1, uint8_t cmd2){}
  void handleSystemCommon(uint8_t cmd1, uint8_t cmd2, uint8_t cmd3){}
  void handleChannelPressure(uint8_t status, uint8_t value){}
  void handlePolyKeyPressure(uint8_t status, uint8_t note, uint8_t value){}
  void handleParameterChange(uint8_t pid, uint16_t value){}

  void setInputChannel(int8_t ch){
    channel = ch;
  }
  static int8_t getChannel(uint8_t status);
private:
  void updateCodecSettings();
  void handleConfigurationCommand(uint8_t* data, uint16_t size);
  void handleFirmwareUploadCommand(uint8_t* data, uint16_t size);
  void handleFirmwareRunCommand(uint8_t* data, uint16_t size);
  void handleFirmwareFlashCommand(uint8_t* data, uint16_t size);
  void handleFirmwareStoreCommand(uint8_t* data, uint16_t size);
};

#endif /* _MidiHandler_h_ */
