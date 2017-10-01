#include <cstddef>
#include <string.h>
#include "Patch.h"
#include "device.h"
#include "ProgramVector.h"
#include "PatchProcessor.h"
#include "basicmaths.h"
#include "main.h"

AudioBuffer::~AudioBuffer(){}

PatchProcessor* getInitialisingPatchProcessor();

Patch::Patch(){}

Patch::~Patch(){}

void Patch::registerParameter(PatchParameterId pid, const char* name){
  if(getProgramVector()->registerPatchParameter != NULL)
    getProgramVector()->registerPatchParameter(pid, name);
}

float Patch::getSampleRate(){
  return getProgramVector()->audio_samplingrate;
}

int Patch::getBlockSize(){
  return getProgramVector()->audio_blocksize;
}

float Patch::getParameterValue(PatchParameterId pid){
  //  return getInitialisingPatchProcessor()->getParameterValue(pid);
  // if(pid < getProgramVector()->parameters_size)
  if(pid < getProgramVector()->parameters_size){
    if(getProgramVector()->hardware_version == OWL_MODULAR_HARDWARE && pid < 4){
      return (4095 - getProgramVector()->parameters[pid])/4096.0f;
    }else{
      return getProgramVector()->parameters[pid]/4096.0f;
    }
  }
  return 0.0f;
}

void Patch::setParameterValue(PatchParameterId pid, float value){
  if(getProgramVector()->hardware_version == OWL_MODULAR_HARDWARE && pid < 4)
    doSetPatchParameter(pid, (4095 - value)*4096.0f);
  else
    doSetPatchParameter(pid, value*4096.0f);
  // if(pid < getProgramVector()->parameters_size){
  //   if(getProgramVector()->hardware_version == OWL_MODULAR_HARDWARE && pid < 4){
  //     getProgramVector()->parameters[pid] = (4095 - value)*4096.0f;
  //   }else{
  //     getProgramVector()->parameters[pid] = value*4096.0f;
  //   }
  // }
}

void Patch::setButton(PatchButtonId bid, uint16_t value, uint16_t samples){
  doSetButton(bid, value, samples);
}

bool Patch::isButtonPressed(PatchButtonId bid){
  return getProgramVector()->buttons & (1<<bid);
}

int Patch::getSamplesSinceButtonPressed(PatchButtonId bid){
  // deprecated
  return 0;
}

AudioBuffer* Patch::createMemoryBuffer(int channels, int samples){
  return AudioBuffer::create(channels, samples);
}

#define DWT_CYCCNT ((volatile unsigned int *)0xE0001004)

float Patch::getElapsedBlockTime(){
  return (*DWT_CYCCNT)/getBlockSize()/3500.0;
}

int Patch::getElapsedCycles(){
  return *DWT_CYCCNT;
}

#include "MemoryBuffer.hpp"
AudioBuffer* AudioBuffer::create(int channels, int samples){
  return new ManagedMemoryBuffer(channels, samples);
}

FloatParameter Patch::getParameter(const char* name, float defaultValue){
  return getFloatParameter(name, 0.0f, 1.0f, defaultValue, 0.0f, 0.0f, LIN);
}

FloatParameter Patch::getFloatParameter(const char* name, float min, float max, float defaultValue, float lambda, float delta, float skew){
  return getInitialisingPatchProcessor()->getParameter(name, min, max, defaultValue, lambda, delta, skew);
}

IntParameter Patch::getIntParameter(const char* name, int min, int max, int defaultValue, float lambda, float delta, float skew){
  return getInitialisingPatchProcessor()->getParameter(name, min, max, defaultValue, lambda, delta, skew);
}

const float Patch::EXP = 0.5;
const float Patch::LIN = 1.0;
const float Patch::LOG = 2.0;
const uint16_t Patch::ON = 4095;
const uint16_t Patch::OFF = 0;
