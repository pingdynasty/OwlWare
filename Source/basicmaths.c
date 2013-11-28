#include "basicmaths.h"

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
