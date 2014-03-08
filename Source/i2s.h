#ifndef __i2s__
#define __i2s__

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

   void I2S_Block_Init(void);
   void I2S_Enable();
   void I2S_Block_Run(uint32_t txAddr, uint32_t rxAddr, uint32_t Size);
   void I2S_Pause();
   void I2S_Resume();
   void I2S_Disable();
   extern void audioCallback(uint16_t *src, uint16_t *dst, uint16_t sz);

#ifdef __cplusplus
}
#endif

#endif

