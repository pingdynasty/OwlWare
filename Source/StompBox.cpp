#include "StompBox.h"
#include "owlcontrol.h"
#include "ApplicationSettings.h"
#include "CodecController.h"
#include "PatchProcessor.h"
#include "device.h"
#include <cstddef>

extern PatchProcessor* getPatchProcessor(); // defined in Owl.cpp; todo: mv

AudioBuffer::~AudioBuffer(){}

Patch::Patch(){
  processor = getPatchProcessor();
}

Patch::~Patch(){}

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
