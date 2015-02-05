#ifndef __BKPSRAM_H
#define __BKPSRAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

  void* BKPSRAM_GetMemoryAddress();
  uint32_t BKPSRAM_GetMemorySize();
  void BKPSRAM_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __BKPSRAM_H */
