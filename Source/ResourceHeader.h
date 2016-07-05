#ifndef __RESOURCE_HEADER_H
#define __RESOURCE_HEADER_H

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

   struct ResourceHeader {
     uint32_t magic; // 0xDADADEED
     uint32_t size;
     char name[24];
   };

#ifdef __cplusplus
}
#endif

#endif /* __RESOURCE_HEADER_H */
