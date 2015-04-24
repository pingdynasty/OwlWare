#ifndef __SAMPLEBUFFER_H__
#define __SAMPLEBUFFER_H__

#include <stdint.h>
#include <string.h>
#include "StompBox.h"
#include "device.h"
#include "arm_math.h"

class SampleBuffer : public AudioBuffer {
protected:
  float left[AUDIO_MAX_BLOCK_SIZE];
  float right[AUDIO_MAX_BLOCK_SIZE];
  uint16_t size;
public:
  void split(int16_t* input, uint16_t blocksize){
#if AUDIO_BITDEPTH == 16
    size = blocksize;
    float* l = left;
    float* r = right;
    uint32_t blkCnt = size >> 1u;
    while(blkCnt > 0u){
      *l++ = ((float)*input++) / 32768.0f;
      *r++ = ((float)*input++) / 32768.0f;
      *l++ = ((float)*input++) / 32768.0f;
      *r++ = ((float)*input++) / 32768.0f;
      blkCnt--;
    }
#else /* AUDIO_BITDEPTH != 16 */
    size = blocksize;
#ifdef AUDIO_BIGEND
    float* l = left;
    float* r = right;
    uint32_t blkCnt = size;
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
#else /* AUDIO_BIGEND */
    float* l = left;
    float* r = right;
    uint32_t blkCnt = size>>1;
    while(blkCnt > 0u){
      *l++ = (int32_t)((*input++)|(*++input)<<16) / 2147483648.0f;
      *r++ = (int32_t)((*input++)|(*++input)<<16) / 2147483648.0f;
      *l++ = (int32_t)((*input++)|(*++input)<<16) / 2147483648.0f;
      *r++ = (int32_t)((*input++)|(*++input)<<16) / 2147483648.0f;
      blkCnt--;
    }
#endif /* AUDIO_BIGEND */
#endif /* AUDIO_BITDEPTH != 16 */
  }
  void comb(int16_t* output){
#if AUDIO_BITDEPTH == 16
    float* l = left;
    float* r = right;
    uint32_t blkCnt = size >> 1u;
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
#else /* AUDIO_BITDEPTH != 16 */
#ifdef AUDIO_BIGEND
    float* l = left;
    float* r = right;
    uint32_t blkCnt = size;
    int16_t* dst = output;
    int32_t qint;
    while(blkCnt > 0u){
#ifdef AUDIO_SATURATE_SAMPLES
      qint = clip_q63_to_q31((q63_t)(*l++ * 2147483648.0f));
      *dst++ = qint >> 16;
      *dst++ = qint & 0xffff;
      qint = clip_q63_to_q31((q63_t)(*r++ * 2147483648.0f));
      *dst++ = qint >> 16;
      *dst++ = qint & 0xffff;
#else /* AUDIO_SATURATE_SAMPLES */
      qint = *l++ * 2147483648.0f;
      *dst++ = qint >> 16;
      *dst++ = qint & 0xffff;
      qint = *r++ * 2147483648.0f;
      *dst++ = qint >> 16;
      *dst++ = qint & 0xffff;
#endif /* AUDIO_SATURATE_SAMPLES */
      blkCnt--;
    }
#else /* AUDIO_BIGEND */
    // todo: test if this works in big-endian on ARM
    float* l = left;
    float* r = right;
    uint32_t blkCnt = size>>1;
    int32_t* dst = (int32_t*)output;
    while(blkCnt > 0u){
      *dst++ = *l++ * 2147483648.0f;
      *dst++ = *r++ * 2147483648.0f;
      *dst++ = *l++ * 2147483648.0f;
      *dst++ = *r++ * 2147483648.0f;
      blkCnt--;
    }
#endif /* AUDIO_BIGEND */
#endif /* AUDIO_BITDEPTH == 16 */
  }
  void clear(){
    memset(left, 0, getSize()*sizeof(float));
    memset(right, 0, getSize()*sizeof(float));
  }
  inline float* getSamples(int channel){
    return channel == 0 ? left : right;
  }
  inline int getChannels(){
    return AUDIO_CHANNELS;
  }
  // void setSize(uint16_t sz){
  // // size is set by split()
  //   if(sz <= AUDIO_MAX_BLOCK_SIZE)
  //     size = sz;
  // }
  inline int getSize(){
    return size;
  }
};

#endif // __SAMPLEBUFFER_H__
