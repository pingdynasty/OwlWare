#include "StompBox.h"
#include <string.h>
#include <stdlib.h>

class MemoryBuffer : public AudioBuffer {
protected:
  float* buffer;
  int channels;
  int size;
public:
  MemoryBuffer(float* buf, int ch, int sz): buffer(buf), channels(ch), size(sz) {}
  virtual ~MemoryBuffer(){}
  void clear(){
    memset(buffer, 0, size*channels*sizeof(float));
  }
  float* getSamples(int channel){
    // assert_param(channel < channels);
    return buffer+channel*size;
  }
  int getChannels(){
    return channels;
  }
  int getSize(){
    return size;
  }
};

class ManagedMemoryBuffer : public MemoryBuffer {
public:
  ManagedMemoryBuffer(int ch, int sz) :
    // MemoryBuffer((float*)malloc(ch*sz*sizeof(float)), ch, sz) {
    // MemoryBuffer(new float*[ch*sz], ch, sz) {
    MemoryBuffer(new float[ch*sz], ch, sz) {
    if(buffer == NULL){
      channels = 0;
      size = 0;
    }
  }
  ~ManagedMemoryBuffer(){
    // free(buffer);
    delete buffer;
  }
};
