#ifndef __ApplicationSettings_H__
#define __ApplicationSettings_H__

#include <inttypes.h>
#include "stm32f4xx.h"

enum I2SProtocol {
  I2S_PROTOCOL_PHILIPS = I2S_Standard_Phillips,
  I2S_PROTOCOL_MSB = I2S_Standard_MSB,
  I2S_PROTOCOL_LSB = I2S_Standard_LSB
};

enum I2SFormat {
  I2S_FORMAT_16bit = I2S_DataFormat_16b,
  I2S_FORMAT_24bit = I2S_DataFormat_24b,
  I2S_FORMAT_32bit = I2S_DataFormat_32b
};

enum PatchMode {
  PATCHMODE_SINGLE,
  PATCHMODE_DUAL,
  PATCHMODE_SERIES,
  PATCHMODE_PARALLEL
};

class ApplicationSettings {
public:
  uint8_t checksum;
  bool audio_codec_master;
  I2SFormat audio_codec_format;
  I2SProtocol audio_codec_protocol;
  uint32_t audio_samplingrate;
  uint8_t audio_bitdepth;
  uint16_t audio_blocksize;
  uint8_t program_index;
  uint8_t inputGainLeft;
  uint8_t inputGainRight;
  uint8_t outputGainLeft;
  uint8_t outputGainRight;
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
