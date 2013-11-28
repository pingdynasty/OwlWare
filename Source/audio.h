#ifndef __audio__
#define __audio__

/** 
 * Audio utility routines by Eric Brombaugh
 * http://ebrombaugh.studionebula.com
 */

#ifdef __cplusplus
 extern "C" {
#endif

/**
 * @brief  Split interleaved stereo into two separate buffers
 * @param  sz -  samples per input buffer (divisible by 2)
 * @param  src - pointer to source buffer
 * @param  ldst - pointer to left dest buffer (even samples)
 * @param  rdst - pointer to right dest buffer (odd samples)
 * @retval none
 */
void audio_split_stereo(int16_t sz, const int16_t *src, int16_t *ldst, int16_t *rdst){
  while(sz){
    *ldst++ = *src++;
    sz--;
    *rdst++ = *src++;
    sz--;
  }
}

/**
 * @brief  combine two separate buffers into interleaved stereo
 * @param  sz -  samples per output buffer (divisible by 2)
 * @param  dst - pointer to source buffer
 * @param  lsrc - pointer to left dest buffer (even samples)
 * @param  rsrc - pointer to right dest buffer (odd samples)
 * @retval none
 */
void audio_comb_stereo(int16_t sz, int16_t *dst, const int16_t *lsrc, const int16_t *rsrc){
  while(sz){
      *dst++ = *lsrc++;
      sz--;
      *dst++ = *rsrc++;
      sz--;
    }
}

/**
 * @brief  morph a to b into destination
 * @param  sz -  samples per buffer
 * @param  dst - pointer to source buffer
 * @param  asrc - pointer to dest buffer
 * @param  bsrc - pointer to dest buffer
 * @param  morph - float morph coeff. 0 = a, 1 = b
 * @retval none
 */
void audio_morph(int16_t sz, int16_t *dst, const int16_t *asrc, const int16_t *bsrc,
		 float morph) {
  float morph_inv = 1.0 - morph, f_sum;
  int32_t sum;
	
  while(sz--){
      f_sum = (float)*asrc++ * morph_inv + (float)*bsrc++ * morph;
      sum = f_sum;
#if 0
      sum = sum > 32767 ? 32767 : sum;
      sum = sum < -32768 ? -32768 : sum;
#else
      __asm("ssat %[dst], #16, %[src]" : [dst] "=r" (sum) : [src] "r" (sum));
#endif
      /* save to destination */
      *dst++ = sum;
    }
}


/* /\** */
/*  * @brief  Split interleaved stereo into two separate buffers */
/*  * @param  sz -  samples per input buffer (divisible by 2) */
/*  * @param  src - pointer to source buffer */
/*  * @param  ldst - pointer to left dest buffer (even samples) */
/*  * @param  rdst - pointer to right dest buffer (odd samples) */
/*  * @retval none */
/*  *\/ */
/* void audio_split_stereo(int32_t sz, const int32_t *src, int32_t *ldst, int32_t *rdst){ */
/*   while(sz){ */
/*     *ldst++ = *src++; */
/*     sz--; */
/*     *rdst++ = *src++; */
/*     sz--; */
/*   } */
/* } */

/* /\** */
/*  * @brief  combine two separate buffers into interleaved stereo */
/*  * @param  sz -  samples per output buffer (divisible by 2) */
/*  * @param  dst - pointer to source buffer */
/*  * @param  lsrc - pointer to left dest buffer (even samples) */
/*  * @param  rsrc - pointer to right dest buffer (odd samples) */
/*  * @retval none */
/*  *\/ */
/* void audio_comb_stereo(int32_t sz, int32_t *dst, const int32_t *lsrc, const int32_t *rsrc){ */
/*   while(sz){ */
/*       *dst++ = *lsrc++; */
/*       sz--; */
/*       *dst++ = *rsrc++; */
/*       sz--; */
/*     } */
/* } */

/* /\** */
/*  * @brief  morph a to b into destination */
/*  * @param  sz -  samples per buffer */
/*  * @param  dst - pointer to source buffer */
/*  * @param  asrc - pointer to dest buffer */
/*  * @param  bsrc - pointer to dest buffer */
/*  * @param  morph - float morph coeff. 0 = a, 1 = b */
/*  * @retval none */
/*  *\/ */
/* void audio_morph(int32_t sz, int32_t *dst, const int32_t *asrc, const int32_t *bsrc, */
/* 		 float morph) { */
/*   float morph_inv = 1.0 - morph, f_sum; */
/*   int32_t sum; */
	
/*   while(sz--){ */
/*       f_sum = (float)*asrc++ * morph_inv + (float)*bsrc++ * morph; */
/*       sum = f_sum; */
/* #if 0 */
/*       sum = sum > 32767 ? 32767 : sum; */
/*       sum = sum < -32768 ? -32768 : sum; */
/* #else */
/*       __asm("ssat %[dst], #16, %[src]" : [dst] "=r" (sum) : [src] "r" (sum)); */
/* #endif */
/*       /\* save to destination *\/ */
/*       *dst++ = sum; */
/*     } */
/* } */

#ifdef __cplusplus
}
#endif

#endif

