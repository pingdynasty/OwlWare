#include <cstddef>
#include <string.h>
#include "StompBox.h"
#include "owlcontrol.h"
#include "device.h"
#include "ProgramVector.h"
#include "PatchProcessor.h"
// #include "PatchController.h"

AudioBuffer::~AudioBuffer(){}

PatchProcessor* getInitialisingPatchProcessor();

Patch::Patch() : processor(getInitialisingPatchProcessor()){
}

Patch::~Patch(){}

void Patch::registerParameter(PatchParameterId pid, const char* name, const char* description){
  if(getProgramVector()->registerPatchParameter != NULL)
    getProgramVector()->registerPatchParameter(pid, name);
}

double Patch::getSampleRate(){
  return getProgramVector()->audio_samplingrate;
}

int Patch::getBlockSize(){
  return getProgramVector()->audio_blocksize;
}

float Patch::getParameterValue(PatchParameterId pid){
  return processor->getParameterValue(pid);
  // if(pid < getProgramVector()->parameters_size)
  //   return getProgramVector()->parameters[pid]/4096.0f;
  // return 0.0;
}

AudioBuffer* Patch::createMemoryBuffer(int channels, int samples){
  return processor->createMemoryBuffer(channels, samples);
  // MemoryBuffer* buf = new ManagedMemoryBuffer(channels, size);
  // if(buf == NULL)
  //   return NULL;
  // buffers[bufferCount++] = buf;
  // buf->clear();
  // return buf;
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
