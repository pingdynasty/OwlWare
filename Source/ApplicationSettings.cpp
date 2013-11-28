#include "ApplicationSettings.h"
#include "eepromcontrol.h"
#include "device.h"

#define APPLICATION_SETTINGS_OFFSET 1024

void ApplicationSettings::init(){
  if(settingsInFlash())
    loadFromFlash();
  else
    reset();
}

bool ApplicationSettings::settingsInFlash(){
  checksum = sizeof(*this);
  return eeprom_read_byte(APPLICATION_SETTINGS_OFFSET) == checksum;
}

void ApplicationSettings::reset(){
  audio_codec_master = AUDIO_CODEC_MASTER;
  audio_codec_format = AUDIO_FORMAT;
  audio_codec_protocol = AUDIO_PROTOCOL;
  audio_samplingrate = AUDIO_SAMPLINGRATE;
  audio_bitdepth = AUDIO_BITDEPTH;
  active_patch = 0;
  patch_a = 0;
  patch_b = 1;
  midi_channel = MIDI_CHANNEL;
}

void ApplicationSettings::loadFromFlash(){
  eeprom_read_block(APPLICATION_SETTINGS_OFFSET, (uint8_t*)this, sizeof(*this));
}

void ApplicationSettings::saveToFlash(){
  eeprom_write_block(APPLICATION_SETTINGS_OFFSET, (uint8_t*)this, sizeof(*this));
}
