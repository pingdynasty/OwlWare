#ifndef __basicmaths_h__
#define __basicmaths_h__

#define _USE_MATH_DEFINES
/* Definitions of useful mathematical constants
 * M_E        - e
 * M_LOG2E    - log2(e)
 * M_LOG10E   - log10(e)
 * M_LN2      - ln(2)
 * M_LN10     - ln(10)
 * M_PI       - pi
 * M_PI_2     - pi/2
 * M_PI_4     - pi/4
 * M_1_PI     - 1/pi
 * M_2_PI     - 2/pi
 * M_2_SQRTPI - 2/sqrt(pi)
 * M_SQRT2    - sqrt(2)
 * M_SQRT1_2  - 1/sqrt(2)
 */
#include <math.h>
#include "arm_math.h" 

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif /* min */
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif /* max */
#ifndef abs
#define abs(x) ((x)>0?(x):-(x))
#endif /* abs */

// todo: see
// http://www.hxa.name/articles/content/fast-pow-adjustable_hxa7241_2007.html
// http://www.finesse.demon.co.uk/steven/sqrt.html
// http://www.keil.com/forum/7934/
// http://processors.wiki.ti.com/index.php/ARM_compiler_optimizations

float fastPow(float a, float b);

/* #define pow(x, y) fastPow(x, y) */
/* #define powf(x, y) fastPow(x, y) */
#define sin(x) arm_sin_f32(x)
#define sinf(x) arm_sin_f32(x)
#define cos(x) arm_cos_f32(x)
#define cosf(x) arm_cos_f32(x)

#endif // __basicmaths_h__
