#include "basicmaths.h"
#include <stdint.h>

float fastPow(float a, float b){
  /* Taken from
   * http://martin.ankerl.com/2007/10/04/optimized-pow-approximation-for-java-and-c-c/
   */
  union {
    float d;
    int x[2];
  } u = { a };
  u.x[1] = (int)(b * (u.x[1] - 1072632447) + 1072632447);
  u.x[0] = 0;
  return u.d;
}

// Faster approximations to sqrt()
//From http://ilab.usc.edu/wiki/index.php/Fast_Square_Root
//The faster the routine, the more error in the approximation.

// Log Base 2 Approximation
// 5 times faster than sqrt()

float fastsqrt1( float x ) {
  union { int32_t i; float x; } u;
  u.x = x;
  u.i = ((int32_t)1<<29) + (u.i >> 1) - ((int32_t)1<<22); 
  return u.x;
}

// Log Base 2 Approximation with one extra Babylonian Step
// 2 times faster than sqrt()

float fastsqrt2( float x ) {
  float v=fastsqrt1( x );
  v = 0.5f * (v + x/v); // One Babylonian step
  return v;
}

// Log Base 2 Approximation with two extra Babylonian Steps
// 50% faster than sqrt()

float fastsqrt3( float x ) {
  float v=fastsqrt1( x );
  v =v + x/v;
  v = 0.25f* v + x/v; // Two Babylonian steps
  return v;
}
