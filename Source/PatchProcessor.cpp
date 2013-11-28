#include "PatchProcessor.h"
#include "MemoryBuffer.hpp"
#include "device.h"
#include "basicmaths.h"

#ifdef EXTERNAL_SRAM
#define BUFFER_LENGTH 262144
static float extbuffer[BUFFER_LENGTH] EXT;
#else /* no external SRAM */
#define BUFFER_LENGTH 16384
static float extbuffer[BUFFER_LENGTH];
#endif
static int extpos = 0;

PatchProcessor::PatchProcessor(Patch* p) : patch(p), bufferCount(0) {
}

PatchProcessor::~PatchProcessor(){
  for(int i=0; i<bufferCount; ++i){
    extpos -= buffers[i]->getSize() * buffers[i]->getChannels();
    delete buffers[i];
  }
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

// float PatchProcessor::getParameterValue(PatchParameterId pid){
//   return patch->getParameterValue(pid);
// }

// int PatchProcessor::getBlockSize(){
//   return patch->getBlockSize();
// }

// double PatchProcessor::getSampleRate(){
//   return patch->getSampleRate();
// }

// void PatchProcessor::processAudio(AudioBuffer& buffer){
//   patch->processAudio(buffer);
// }
