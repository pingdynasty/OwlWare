#ifndef __SAMPLEBUFFER_H__
#define __SAMPLEBUFFER_H__

#include <stdint.h>
#include "StompBox.h"
// #include "audio.h"
#include "device.h"

// template<int bits, bool saturate, int size, float* left, float* right>
class SampleBuffer : public AudioBuffer {
protected:
  float left[AUDIO_BLOCK_SIZE];
  float right[AUDIO_BLOCK_SIZE];
public:
  SampleBuffer(){}
  virtual void split(uint16_t* input) = 0;
  virtual void comb(uint16_t* input) = 0;
  float* getSamples(int channel){
    return channel == 0 ? left : right;
  }
  int getChannels(){
    return AUDIO_CHANNELS;
  }
  int getSize(){
    return AUDIO_BLOCK_SIZE;
  }
};

class SampleBuffer32 : public SampleBuffer {
public:
  void split(uint16_t* input){
#ifdef AUDIO_BIGEND
    float* l = left;
    float* r = right;
    uint32_t blkCnt = AUDIO_BLOCK_SIZE;
    int32_t qint;
    while(blkCnt > 0u){
      qint = (*input++)<<16;
      qint |= *input++;
      *l++ = qint / 2147483648.0f;
      qint = (*input++)<<16;
      qint |= *input++;
      *r++ = qint / 2147483648.0f;
      // *l++ = (int32_t)((*input++)<<16|*++input) / 2147483648.0f;
      // *r++ = (int32_t)((*input++)<<16|*++input) / 2147483648.0f;
      // *l++ = (int32_t)((*input++)<<16|*++input) / 2147483648.0f;
      // *r++ = (int32_t)((*input++)<<16|*++input) / 2147483648.0f;
      blkCnt--;
    }
#else
    float* l = left;
    float* r = right;
    uint32_t blkCnt = AUDIO_BLOCK_SIZE>>1;
    while(blkCnt > 0u){
      *l++ = (int32_t)((*input++)|(*++input)<<16) / 2147483648.0f;
      *r++ = (int32_t)((*input++)|(*++input)<<16) / 2147483648.0f;
      *l++ = (int32_t)((*input++)|(*++input)<<16) / 2147483648.0f;
      *r++ = (int32_t)((*input++)|(*++input)<<16) / 2147483648.0f;
      blkCnt--;
    }
#endif
  }

  void comb(uint16_t* output){
#ifdef AUDIO_BIGEND
    float* l = left;
    float* r = right;
    uint32_t blkCnt = AUDIO_BLOCK_SIZE;
    uint16_t* dst = output;
    int32_t qint;
    while(blkCnt > 0u){
#ifdef AUDIO_SATURATE_SAMPLES
      qint = clip_q63_to_q31((q63_t)(*l++ * 2147483648.0f));
      *dst++ = qint >> 16;
      *dst++ = qint & 0xffff;
      qint = clip_q63_to_q31((q63_t)(*r++ * 2147483648.0f));
      *dst++ = qint >> 16;
      *dst++ = qint & 0xffff;
#else
      qint = *l++ * 2147483648.0f;
      *dst++ = qint >> 16;
      *dst++ = qint & 0xffff;
      qint = *r++ * 2147483648.0f;
      *dst++ = qint >> 16;
      *dst++ = qint & 0xffff;
#endif /* AUDIO_SATURATE_SAMPLES */
      blkCnt--;
    }
#else
    // todo: test if this works in big-endian on ARM
    float* l = left;
    float* r = right;
    uint32_t blkCnt = AUDIO_BLOCK_SIZE>>1;
    int32_t* dst = (int32_t*)output;
    while(blkCnt > 0u){
      *dst++ = *l++ * 2147483648.0f;
      *dst++ = *r++ * 2147483648.0f;
      *dst++ = *l++ * 2147483648.0f;
      *dst++ = *r++ * 2147483648.0f;
      blkCnt--;
    }
#endif
  }
};

class SampleBuffer16 : public SampleBuffer {
public:
  void split(uint16_t* input){
    float* l = left;
    float* r = right;
    uint32_t blkCnt = AUDIO_BLOCK_SIZE >> 1u;
    while(blkCnt > 0u){
      *l++ = ((float)*input++) / 32768.0f;
      *r++ = ((float)*input++) / 32768.0f;
      *l++ = ((float)*input++) / 32768.0f;
      *r++ = ((float)*input++) / 32768.0f;
      blkCnt--;
    }
  }
  void comb(uint16_t* output){
    float* l = left;
    float* r = right;
    uint32_t blkCnt = AUDIO_BLOCK_SIZE >> 1u;
    while(blkCnt > 0u){
#ifdef AUDIO_SATURATE_SAMPLES
      *output++ = (q15_t)__SSAT((q31_t)((*l++) * 32768.0f), 16);
      *output++ = (q15_t)__SSAT((q31_t)((*r++) * 32768.0f), 16);
      *output++ = (q15_t)__SSAT((q31_t)((*l++) * 32768.0f), 16);
      *output++ = (q15_t)__SSAT((q31_t)((*r++) * 32768.0f), 16);
#else
      *output++ = (q15_t)((*l++) * 32768.0f);
      *output++ = (q15_t)((*r++) * 32768.0f);
      *output++ = (q15_t)((*l++) * 32768.0f);
      *output++ = (q15_t)((*r++) * 32768.0f);
#endif
      blkCnt--;
    }
  }

};

#endif // __SAMPLEBUFFER_H__
