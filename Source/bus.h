#ifndef __BUS_H
#define __BUS_H

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

   void setupBus();
   void UART4_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* __BUS_H */
