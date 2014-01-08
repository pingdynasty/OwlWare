#include "PatchProcessor.h"
#include "PatchRegistry.h"
#include "MemoryBuffer.hpp"
#include "device.h"
#include "basicmaths.h"
#include <string.h>

#ifdef EXTERNAL_SRAM
#define BUFFER_LENGTH 262144
static float extbuffer[BUFFER_LENGTH] EXT;
#else /* no external SRAM */
#define BUFFER_LENGTH 16384
static float extbuffer[BUFFER_LENGTH];
#endif
static int extpos = 0;

PatchProcessor::PatchProcessor(uint8_t i) : patch(NULL), index(i), bufferCount(0) {
  patch = registry.create(index);
  memset(parameters, 0, sizeof(parameters));
}

PatchProcessor::~PatchProcessor(){
  for(int i=0; i<bufferCount; ++i){
    extpos -= buffers[i]->getSize() * buffers[i]->getChannels();
    delete buffers[i];
  }
  delete patch;
}

AudioBuffer* PatchProcessor::createMemoryBuffer(int channels, int size){
  // assert_param(bufferCount < MAX_BUFFERS_PER_PATCH];
  float* buffer = extbuffer+extpos;
  size = min(size, BUFFER_LENGTH-extpos);
  extpos += size;
  size /= channels;
  MemoryBuffer* buf = new MemoryBuffer(buffer, channels, size);
  buffers[bufferCount++] = buf;
  return buf;
}

float PatchProcessor::getParameterValue(PatchParameterId pid){
  return parameters[pid]/4096.0;
  // return getAnalogValue(pid)/4096.0;
//   return patch->getParameterValue(pid);
}

void PatchProcessor::setParameters(uint16_t *params){
  memcpy(parameters, params, sizeof parameters);
}

void PatchProcessor::process(AudioBuffer& buffer){
  patch->processAudio(buffer);
}

// int PatchProcessor::getBlockSize(){
//   return patch->getBlockSize();
// }

// double PatchProcessor::getSampleRate(){
//   return patch->getSampleRate();
// }
