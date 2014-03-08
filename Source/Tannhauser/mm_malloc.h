/*
 * This file is intended for systems that do not include mm_malloc.h (such as emscripten).
 * It simply redefines mm_malloc as malloc, ignoring the alignment argument.
 */

#ifndef _MM_MALLOC_H_
#define _MM_MALLOC_H_

//#include <stdlib.h>

#define _mm_malloc(_numBytes, _alignment) malloc(_numBytes)
#define _mm_free(_ptr) free(_ptr)

#endif // _MM_MALLOC_H_
