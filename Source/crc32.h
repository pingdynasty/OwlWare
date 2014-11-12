#ifndef __CRC32_H
#define __CRC32_H

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
 extern "C" {
#endif

   uint32_t crc32(const void *buf, size_t size, uint32_t crc);

#ifdef __cplusplus
}
#endif

#endif /* __CRC32_H */
