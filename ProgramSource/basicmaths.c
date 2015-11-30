#include "basicmaths.h"
#include <stdint.h>

float arm_sqrtf(float in){
  float out;
#ifdef ARM_CORTEX
  arm_sqrt_f32(in, &out);
#else
  out=sqrtf(in);
#endif
  return out;
}

float fastpowf(float a, float b){
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
  float v = fastsqrt1( x );
  v =v + x/v;
  v = 0.25f* v + x/v; // Two Babylonian steps
  return v;
}


/* ----------------------------------------------------------------------
** Fast approximation to the log2() function.  It uses a two step
** process.  First, it decomposes the floating-point number into
** a fractional component F and an exponent E.  The fraction component
** is used in a polynomial approximation and then the exponent added
** to the result.  A 3rd order polynomial is used and the result
** when computing db20() is accurate to 7.984884e-003 dB.
** http://community.arm.com/thread/6741
** ------------------------------------------------------------------- */
const float log2f_approx_coeff[4] = {1.23149591368684f, -4.11852516267426f, 6.02197014179219f, -3.13396450166353f};

float fastlog2f(float X){
  const float *C = &log2f_approx_coeff[0];
  float Y;
  float F;
  int E;
  // This is the approximation to log2()
  F = frexpf(fabsf(X), &E);
  //  Y = C[0]*F*F*F + C[1]*F*F + C[2]*F + C[3] + E;
  Y = *C++;
  Y *= F;
  Y += (*C++);
  Y *= F;
  Y += (*C++);
  Y *= F;
  Y += (*C++);
  Y += E;
  return(Y);
}
