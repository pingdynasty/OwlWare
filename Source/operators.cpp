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

// Static initialisation thread safety guards
// Returns 1 if the initialization is not yet complete; 0 otherwise.
extern "C" int __cxa_guard_acquire ( __int64_t *guard_object ){ return !*(char *)(guard_object); }
// Sets the first byte of the guard object to a non-zero value.
extern "C" void __cxa_guard_release ( __int64_t *guard_object ){ *(char *)guard_object = 1; }
extern "C" void __cxa_guard_abort ( __int64_t *guard_object ){}
