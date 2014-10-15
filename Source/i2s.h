#ifndef __i2s__
#define __i2s__

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

   void I2S_Block_Init(uint16_t *txAddr, uint16_t *rxAddr, uint16_t size);
   void I2S_Enable();
   void I2S_Run();
   void I2S_Pause();
   void I2S_Resume();
   void I2S_Disable();
   extern void audioCallback(uint16_t *src, uint16_t *dst, uint16_t sz);

#ifdef __cplusplus
}
#endif

#endif

