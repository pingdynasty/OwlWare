#ifndef __SHARED_MEMORY_H
#define __SHARED_MEMORY_H

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

   typedef struct {
     uint8_t checksum;
     uint8_t audio_status;
     uint16_t* audio_input;
     uint16_t* audio_output;
     uint8_t audio_bitdepth;
     uint16_t audio_blocksize;
     uint32_t audio_samplingrate;
     uint16_t* parameters;
     uint8_t parameters_size;
     uint8_t error_status;
   } SharedMemory;

__attribute__ ((section (".sharedram")))
static volatile SharedMemory smem;

#ifdef __cplusplus
}
#endif

#endif /* __SHARED_MEMORY_H */
