#include "basicmaths.h"
#include <stdint.h>
#include "FastTrans.h"
#include "device.h"

extern "C"{

// todo: see
// http://www.hxa.name/articles/content/fast-pow-adjustable_hxa7241_2007.html
// http://www.finesse.demon.co.uk/steven/sqrt.html
// http://www.keil.com/forum/7934/
// http://processors.wiki.ti.com/index.php/ARM_compiler_optimizations

static uint32_t r32seed = 33641;

void arm_srand32(uint32_t s){
  r32seed = s;
}

/**
 * generate an unsigned 32bit pseudo-random number using xorshifter algorithm.
 * "Anyone who considers arithmetical methods of producing random digits is, of course, in a state of sin." 
 * -- John von Neumann.
*/
uint32_t arm_rand32(){
  r32seed ^= r32seed << 13;
  r32seed ^= r32seed >> 17;
  r32seed ^= r32seed << 5;
  return r32seed;
}

float arm_sqrtf(float in){
  float out;
#ifdef ARM_CORTEX
  arm_sqrt_f32(in, &out);
#else
  out=sqrtf(in);
#endif
  return out;
}

static char fastPowInitialized = 0;
static FastPow fastPow;
static void initializeFastPow(int fastPrecision){
  fastPow.setup(fastPrecision);
  fastPowInitialized = 1;
}
/* http://stackoverflow.com/questions/6475373/optimizations-for-pow-with-const-non-integer-exponent */
/* http://www.hxa.name/articles/content/fast-pow-adjustable_hxa7241_2007.html */
/* https://hackage.haskell.org/package/approximate-0.2.2.3/src/cbits/fast.c */
  //  union { float d; int x; } u = { a };
  //  u.x = (int)(b * (u.x - 1064866805) + 1064866805);
  //  return u.d;

// New implementation, uses FastPow class
float fastpowf(float a, float b) {
  if(fastPowInitialized == 0){
    initializeFastPow(FASTPOW_PRECISION);
  }
  return fastPow.pow(a,b);
}

float fastpow2f(float b){
  if(fastPowInitialized == 0){
    initializeFastPow(FASTPOW_PRECISION);
  }
  return fastPow.powIlog(1, b);
}

float fastexpf(float b) {
  if(fastPowInitialized == 0){
    initializeFastPow(FASTPOW_PRECISION);
  }
  static float eulerIlog = fastPow.computeIlog(exp(1));
  return fastPow.powIlog(eulerIlog, b);
}

float fastlog2f(float b){
  if(fastPowInitialized == 0){
    initializeFastPow(FASTPOW_PRECISION); //note that here we would be initializing two tables, only to use the log.
  }
  static float ilog = 1/fastPow.log(2);
  return fastPow.log(b) * ilog;
}

float fastlog10f(float b){
  // todo: test!
  if(fastPowInitialized == 0)
    initializeFastPow(FASTPOW_PRECISION);
  static float ilog = 1/fastPow.log(10);
  return fastPow.log(b) * ilog;
}

float fastlogf(float b){
  // todo: test!
  if(fastPowInitialized == 0)
    initializeFastPow(FASTPOW_PRECISION);
  static float ilog = 1/fastPow.log(M_E);
  return fastPow.log(b) * ilog;
}

/* Fast arctan2
 * from http://dspguru.com/dsp/tricks/fixed-point-atan2-with-self-normalization
 */
float fastatan2f(float y, float x){
  const float coeff_1 = M_PI/4;
  const float coeff_2 = 3*M_PI/4;
  float abs_y = fabs(y)+1e-10; // kludge to prevent 0/0 condition
  float r, angle;
  if (x>=0){
    r = (x - abs_y) / (x + abs_y);
    angle = coeff_1 - coeff_1 * r;
  }else{
    r = (x + abs_y) / (abs_y - x);
    angle = coeff_2 - coeff_1 * r;
  }
  if(y < 0)
    return(-angle); // negate if in quad III or IV
  else
    return(angle);
}

} /* extern "C" */
