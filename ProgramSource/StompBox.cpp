#include <cstddef>
#include <string.h>
#include "StompBox.h"
#include "owlcontrol.h"
#include "device.h"
#include "ProgramVector.h"
#include "PatchProcessor.h"
#include "ApplicationSettings.h"
#include "MemoryBuffer.hpp"

AudioBuffer::~AudioBuffer(){}

extern PatchProcessor* getInitialisingPatchProcessor();

Patch::Patch() : processor(getInitialisingPatchProcessor()){
  ASSERT(processor != NULL, "Initialising patch processor can't be NULL");
}

Patch::~Patch(){}

void Patch::registerParameter(PatchParameterId pid, const char* name, const char* description){
  if(getProgramVector()->registerPatchParameter != NULL)
    getProgramVector()->registerPatchParameter(pid, name);
}

double Patch::getSampleRate(){
  // return settings.audio_samplingrate;
  return getProgramVector()->audio_samplingrate;
}

int Patch::getBlockSize(){
  // // return settings.audio_blocksize;
  return getProgramVector()->audio_blocksize;
}

float Patch::getParameterValue(PatchParameterId pid){
  return processor->getParameterValue(pid);
  // if(pid < getProgramVector()->parameters_size)
  //   return getProgramVector()->parameters[pid]/4096.0f;
  // return 0.0;
}

AudioBuffer* Patch::createMemoryBuffer(int channels, int samples){
   MemoryBuffer* buf = new ManagedMemoryBuffer(channels, samples);
   ASSERT(buf != NULL, "malloc failed");
   return buf;
}

void Patch::setButton(PatchButtonId bid, bool pressed){
  // processor->setButton(bid, pressed);
  if(pressed)
    getProgramVector()->buttons |= 1<<bid;
  else
    getProgramVector()->buttons &= ~(1<<bid);
}

bool Patch::isButtonPressed(PatchButtonId bid){
  // return processor->isButtonPressed(bid);
  return getProgramVector()->buttons & (1<<bid);
}

AudioBuffer* AudioBuffer::create(int channels, int samples){
  return new ManagedMemoryBuffer(channels, samples);
}
