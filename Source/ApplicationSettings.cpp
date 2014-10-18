#include "ApplicationSettings.h"
#include "eepromcontrol.h"
#include "device.h"

#define APPLICATION_SETTINGS_OFFSET 0

void ApplicationSettings::init(){
  if(settingsInFlash())
    loadFromFlash();
  else
    reset();
}

void ApplicationSettings::reset(){
  audio_codec_master = AUDIO_CODEC_MASTER;
  audio_codec_format = AUDIO_FORMAT;
  audio_codec_protocol = AUDIO_PROTOCOL;
  audio_samplingrate = AUDIO_SAMPLINGRATE;
  audio_bitdepth = AUDIO_BITDEPTH;
  audio_blocksize = AUDIO_BLOCK_SIZE;
  patch_green = GREEN_PATCH;
  patch_red = RED_PATCH;
  patch_mode = PATCHMODE_DUAL;
  midi_channel = MIDI_CHANNEL;
  inputGainLeft = AUDIO_INPUT_GAIN_LEFT;
  inputGainRight = AUDIO_INPUT_GAIN_RIGHT;
  outputGainLeft = AUDIO_OUTPUT_GAIN_LEFT;
  outputGainRight = AUDIO_OUTPUT_GAIN_RIGHT;
}

bool ApplicationSettings::settingsInFlash(){
  checksum = sizeof(*this);
  return eeprom_read_byte(APPLICATION_SETTINGS_OFFSET) == checksum;
}

void ApplicationSettings::loadFromFlash(){
  eeprom_read_block(APPLICATION_SETTINGS_OFFSET, (uint8_t*)this, sizeof(*this));
  // // todo: remove workaround
  // if(audio_blocksize == 0 || audio_blocksize > AUDIO_MAX_BLOCK_SIZE)
  //   audio_blocksize = AUDIO_MAX_BLOCK_SIZE;
}

void ApplicationSettings::saveToFlash(){
  eeprom_unlock();
  eeprom_erase();
  eeprom_write_block(APPLICATION_SETTINGS_OFFSET, (uint8_t*)this, sizeof(*this));
  eeprom_lock();
}

void ApplicationSettings::clearFlash(){
  eeprom_unlock();
  eeprom_erase();
  eeprom_lock();
}
