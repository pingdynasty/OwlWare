#include "ApplicationSettings.h"
#include "eepromcontrol.h"
#include "device.h"

#define APPLICATION_SETTINGS_ADDR ADDR_FLASH_SECTOR_1
#define APPLICATION_SETTINGS_SECTOR FLASH_Sector_1

void ApplicationSettings::init(){
  checksum = sizeof(*this) ^ 0xffffffff;
  if(settingsInFlash())
    loadFromFlash();
  else
    reset();
}

void ApplicationSettings::reset(){
  audio_codec_master = AUDIO_CODEC_MASTER;
  audio_codec_swaplr = false;
  audio_codec_bypass = false;
  audio_codec_halfspeed = false;
  audio_codec_protocol = AUDIO_PROTOCOL;
  audio_samplingrate = AUDIO_SAMPLINGRATE;
  // audio_bitdepth = AUDIO_BITDEPTH;
  audio_bitdepth = 16; // todo: fix codec 16/24 bit modes
  audio_dataformat = AUDIO_DATAFORMAT;
  audio_blocksize = AUDIO_BLOCK_SIZE;
  inputGainLeft = AUDIO_INPUT_GAIN_LEFT;
  inputGainRight = AUDIO_INPUT_GAIN_RIGHT;
  outputGainLeft = AUDIO_OUTPUT_GAIN_LEFT;
  outputGainRight = AUDIO_OUTPUT_GAIN_RIGHT;
  program_index = DEFAULT_PROGRAM;
  program_change_button = true;
  input_offset = AUDIO_INPUT_OFFSET;
  input_scalar = AUDIO_INPUT_SCALAR;
  output_offset = AUDIO_OUTPUT_OFFSET;
  output_scalar = AUDIO_OUTPUT_SCALAR;
  midi_input_channel = MIDI_INPUT_CHANNEL;
  midi_output_channel = MIDI_OUTPUT_CHANNEL;
}

bool ApplicationSettings::settingsInFlash(){
  return eeprom_read_word(APPLICATION_SETTINGS_ADDR) == checksum;
}

void ApplicationSettings::loadFromFlash(){
  eeprom_read_block(APPLICATION_SETTINGS_ADDR, this, sizeof(*this));
}

void ApplicationSettings::saveToFlash(){
  eeprom_unlock();
  //   if(eeprom_erase(APPLICATION_SETTINGS_ADDR) == 0)
  eeprom_erase_sector(APPLICATION_SETTINGS_SECTOR);
  eeprom_write_block(APPLICATION_SETTINGS_ADDR, this, sizeof(*this));
  eeprom_lock();
}

void ApplicationSettings::clearFlash(){
  eeprom_unlock();
  eeprom_erase_sector(APPLICATION_SETTINGS_SECTOR);
  //  eeprom_erase(APPLICATION_SETTINGS_ADDR);
  eeprom_lock();
}
