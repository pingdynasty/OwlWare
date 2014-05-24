#ifndef __OWL_H__
#define __OWL_H__

#include <inttypes.h>

#ifdef __cplusplus
 extern "C" {
#endif

uint16_t getAnalogValue(uint8_t index);
void audioCallback(uint16_t *src, uint16_t *dst, uint16_t sz);
   
#ifdef __cplusplus
}
#endif

#endif // __OWL_H__
