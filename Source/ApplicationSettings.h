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

class ApplicationSettings {
public:
  uint8_t checksum;
  bool audio_codec_master;
  I2SFormat audio_codec_format;
  I2SProtocol audio_codec_protocol;
  uint32_t audio_samplingrate;
  uint8_t audio_bitdepth;
  uint32_t audio_blocksize;
  uint8_t active_patch;
  uint8_t patch_a;
  uint8_t patch_b;
  uint8_t midi_channel;
public:
  void init();
  void reset();
  bool settingsInFlash();
  void loadFromFlash();
  void saveToFlash();
};

extern ApplicationSettings settings;


/* class ApplicationSettings { */
/*  public: */
/*   float getValue(OpenWareMidiControl pid); */
/*   void saveToFlash(); */
/*   void loadFromFlash(); */
/*   void reset(); */
/* }; */

#endif // __ApplicationSettings_H__
