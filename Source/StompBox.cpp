#include "StompBox.h"
#include "owlcontrol.h"
#include "ApplicationSettings.h"
#include "CodecController.h"
#include "MidiController.h"
#include "OpenWareMidiControl.h"
#include "PatchProcessor.h"
#include "device.h"
#include <cstddef>
#include <string.h>

extern PatchProcessor* getPatchProcessor(); // defined in Owl.cpp; todo: mv

AudioBuffer::~AudioBuffer(){}

Patch::Patch(){
  processor = getPatchProcessor();
}

Patch::~Patch(){}

void Patch::registerParameter(PatchParameterId pid, const char* name, const char* description){
  uint8_t size = strlen(name);
  uint8_t buffer[size+3];
  buffer[0] = SYSEX_PARAMETER_NAME_COMMAND;
  buffer[1] = pid;
  memcpy(buffer+2, name, size+1);
  midi.sendSysEx(buffer, sizeof(buffer));
}

double Patch::getSampleRate(){
  return codec.getSamplingRate();
}

int Patch::getBlockSize(){
  return AUDIO_BLOCK_SIZE;
 // return settings.audio_blocksize;
}

float Patch::getParameterValue(PatchParameterId pid){
  return getAnalogValue(pid)/4096.0;
}

AudioBuffer* Patch::createMemoryBuffer(int channels, int samples){
  return processor->createMemoryBuffer(channels, samples);
}
