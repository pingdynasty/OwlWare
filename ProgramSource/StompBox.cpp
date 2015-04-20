#include <cstddef>
#include <string.h>
#include "StompBox.h"
#include "owlcontrol.h"
#include "device.h"
#include "SharedMemory.h"
#include "PatchProcessor.h"
// #include "PatchController.h"

AudioBuffer::~AudioBuffer(){}

PatchProcessor* getInitialisingPatchProcessor();

Patch::Patch() : processor(getInitialisingPatchProcessor()){
}

Patch::~Patch(){}

void Patch::registerParameter(PatchParameterId pid, const char* name, const char* description){
  if(getSharedMemory()->registerPatchParameter != NULL)
    getSharedMemory()->registerPatchParameter(pid, name);
}

double Patch::getSampleRate(){
  return getSharedMemory()->audio_samplingrate;
}

int Patch::getBlockSize(){
  // audio_blocksize is for both channels
  return getSharedMemory()->audio_blocksize/2;
}

float Patch::getParameterValue(PatchParameterId pid){
  return processor->getParameterValue(pid);
  // if(pid < getSharedMemory()->parameters_size)
  //   return getSharedMemory()->parameters[pid]/4096.0f;
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
    getSharedMemory()->buttons |= 1<<bid;
  else
    getSharedMemory()->buttons &= ~(1<<bid);
}

bool Patch::isButtonPressed(PatchButtonId bid){
  // return processor->isButtonPressed(bid);
  return getSharedMemory()->buttons & (1<<bid);
}
