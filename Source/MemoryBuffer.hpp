#include "StompBox.h"
#include <string.h>

class MemoryBuffer : public AudioBuffer {
private:
  float* buffer;
  int channels;
  int size;
public:
  MemoryBuffer(float* buf, int ch, int sz): buffer(buf), channels(ch), size(sz) {}
  ~MemoryBuffer(){}
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
