#include <stdlib.h>
// #include <string.h>
// #include "myalloc.h"
// #include <inttypes.h>
// #include "FreeRTOS.h"
// #include <cstddef>
#include "sramalloc.h"

extern "C" void *__gxx_personality_v0;
extern "C" void __cxa_end_cleanup (void);
extern "C" void __cxa_pure_virtual(){}

void * operator new(size_t size) { return sram_alloc(size); }
void * operator new (size_t, void * p) { return p ; }
void * operator new[](size_t size) { return sram_alloc(size); }
void operator delete(void* ptr) { sram_free(ptr); }
void operator delete[](void * ptr) { sram_free(ptr); }

int __errno;

// void * operator new(size_t size) { return pvPortMalloc(size); }
// void * operator new (size_t, void * p) { return p ; }
// void * operator new[](size_t size) { return pvPortMalloc(size); }
// void operator delete(void* ptr) { vPortFree(ptr); }
// void operator delete[](void * ptr) { vPortFree(ptr); }
