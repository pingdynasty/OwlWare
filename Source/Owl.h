#ifndef __OWL_H__
#define __OWL_H__

#include <inttypes.h>
#include "ProgramVector.h"

#ifdef __cplusplus
 extern "C" {
#endif

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

   uint16_t getAnalogValue(uint8_t index);
   uint16_t getParameterValue(uint8_t index);
   void setParameterValue(uint8_t index, uint16_t value);
   void audioCallback(int16_t *src, int16_t *dst);
   bool getButton(uint8_t bid);
   void setButton(uint8_t bid, bool on);   
   void buttonChanged(uint8_t bid, bool on);   
   void updateButtons(uint16_t bid);   
   void setup(); // main OWL setup

#ifdef __cplusplus
}
   void updateProgramVector(ProgramVector*);
#endif

#endif // __OWL_H__
