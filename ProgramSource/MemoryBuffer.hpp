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
  FloatArray getSamples(int channel){
    return FloatArray(buffer+channel*size, size);
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
    MemoryBuffer(new float[ch*sz], ch, sz) {
    if(buffer == NULL){
      channels = 0;
      size = 0;
    }
  }
  ~ManagedMemoryBuffer(){
    delete buffer;
  }
};
