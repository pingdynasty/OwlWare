#include "PatchProcessor.h"
#include "PatchRegistry.h"
#include "MemoryBuffer.hpp"
#include "device.h"
#include <string.h>

#include "MidiController.h"
#include "OpenWareMidiControl.h"

PatchProcessor::PatchProcessor() 
  : patch(NULL), bufferCount(0) {}

PatchProcessor::~PatchProcessor(){
  clear();
}

void PatchProcessor::clear(){
  for(int i=0; i<bufferCount; ++i)
    delete buffers[i];
  delete patch;
  patch = NULL;
}

void PatchProcessor::setPatch(uint8_t patchIndex){
  clear();
  memset(parameterNames, 0, sizeof(parameterNames));
  bufferCount = 0;
  if(patchIndex < registry.getNumberOfPatches())
    index = patchIndex;
  else
    index = 0;
  patch = registry.create(index);
}

void PatchProcessor::registerParameter(PatchParameterId pid, const char* name, const char* description){
  if(pid < NOF_ADC_VALUES)
    parameterNames[pid] = name;
}

const char* PatchProcessor::getParameterName(PatchParameterId pid){
  if(pid < NOF_ADC_VALUES)
    return parameterNames[pid];
  return NULL;
}

AudioBuffer* PatchProcessor::createMemoryBuffer(int channels, int size){
  MemoryBuffer* buf = new ManagedMemoryBuffer(channels, size);
  buffers[bufferCount++] = buf;
  buf->clear();
  return buf;
}

float PatchProcessor::getParameterValue(PatchParameterId pid){
  return parameterValues[pid]/4096.0f;
}

__attribute__ ((section (".coderam")))
void PatchProcessor::setParameterValues(uint16_t *params){
  /* Implements an exponential moving average (leaky integrator) to smooth ADC values
   * y(n) = (1-alpha)*y(n-1) + alpha*y(n)
   * with alpha=0.5, fs=48k, bs=128, then w0 ~= 18hz
   */
  for(int i=0; i<NOF_ADC_VALUES; ++i)
#ifdef EUROOWL
    parameterValues[i] = (parameterValues[i] + 0x1000 - params[i]) >> 1;
#else
    parameterValues[i] = (parameterValues[i] + params[i]) >> 1;
#endif
}

// void PatchProcessor::process(AudioBuffer& buffer){
//   patch->processAudio(buffer);
// }

// int PatchProcessor::getBlockSize(){
//   return patch->getBlockSize();
// }

// double PatchProcessor::getSampleRate(){
//   return patch->getSampleRate();
// }
