#ifndef __ApplicationSettings_H__
#define __ApplicationSettings_H__

#include <inttypes.h>
#include "stm32f4xx.h"

enum I2SProtocol {
  I2S_PROTOCOL_PHILIPS = I2S_Standard_Phillips,
  I2S_PROTOCOL_MSB = I2S_Standard_MSB,
  I2S_PROTOCOL_LSB = I2S_Standard_LSB
};

class ApplicationSettings {
public:
  uint32_t checksum;
  uint32_t audio_samplingrate;
  uint8_t audio_bitdepth;
  uint8_t audio_dataformat;
  uint16_t audio_blocksize;
  uint8_t inputGainLeft;
  uint8_t inputGainRight;
  uint8_t outputGainLeft;
  uint8_t outputGainRight;
  bool audio_codec_master;
  bool audio_codec_swaplr;
  bool audio_codec_bypass;
  bool audio_codec_halfspeed;
  uint8_t audio_codec_protocol;
  uint8_t program_index;
  bool program_change_button;
public:
  void init();
  void reset();
  bool settingsInFlash();
  void loadFromFlash();
  void saveToFlash();
  void clearFlash();
};

extern ApplicationSettings settings;

#endif // __ApplicationSettings_H__
