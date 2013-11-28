#ifndef __CLOCK_H
#define __CLOCK_H

#include <inttypes.h>

#ifdef __cplusplus
 extern "C" {
#endif

void clockSetup();
void delay(uint32_t ms);
uint32_t getSysTicks();

#ifdef __cplusplus
}
#endif

#endif /* __CLOCK_H */
