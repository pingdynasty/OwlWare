#ifndef __CLOCK_H
#define __CLOCK_H

#include <inttypes.h>

#ifdef __cplusplus
 extern "C" {
#endif

   volatile extern uint32_t systicks;

   void clockSetup();
   void delay(uint32_t ms);

/* uint32_t getSysTicks(){ */
/*   return systicks; */
/* } */
#define getSysTicks() (systicks)

#ifdef __cplusplus
}
#endif

#endif /* __CLOCK_H */
