#if _WIN32
    #include <malloc.h>
#else
    #include <mm_malloc.h>
#endif
//#include <stdlib.h>
#include <string.h>

#include "MessageTable.h"

void mTable_init(MessageTable *o, int length) {
	o->length = length;
	o->buffer = (float *) _mm_malloc(length * sizeof(float), 16);
    memset(o->buffer, 0, length * sizeof(float));
}

void mTable_free(MessageTable *o) {
	_mm_free(o->buffer);
}

void mTable_resize(MessageTable *o, int newLength) {
	float *b = realloc(o->buffer, newLength*sizeof(float));
	if (b != o->buffer) {
		// the buffer has been reallocated, ensure that it is on a 16-byte boundary
		if ((((long long) b) & 0xFL) == 0) {
			o->buffer = b;
		} else {
			float *c = (float *) _mm_malloc(newLength*sizeof(float), 16);
			memset(c, 0, newLength*sizeof(float));
			const int min = (o->length <= newLength) ? o->length : newLength;
			memcpy(c, b, min * sizeof(float));
			free(b);
			o->buffer = c;
		}
	}
	o->length = newLength;
}
