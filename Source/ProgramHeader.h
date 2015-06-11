#ifndef __PROGRAM_HEADER_H
#define __PROGRAM_HEADER_H

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

   struct ProgramHeader {
     uint32_t magic;
     uint32_t* linkAddress;
     uint32_t* endAddress;
     uint32_t* jumpAddress;
     uint32_t* stackBegin;
     uint32_t* stackEnd;
     ProgramVector* programVector;
     char programName[24];
   };

#ifdef __cplusplus
}
#endif

#endif /* __PROGRAM_HEADER_H */
