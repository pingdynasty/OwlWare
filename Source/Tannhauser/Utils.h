#ifndef _TANN_UTILS_H_
#define _TANN_UTILS_H_

#if __APPLE__
#include <Accelerate/Accelerate.h>
#endif

static inline int utils_max(int a, int b) { return (a > b) ? a : b; }

static inline int utils_min(int a, int b) { return (a < b) ? a : b; }

static inline void utils_clearBuffer(float *b, const int n) {
#if __APPLE__
	vDSP_vclr(b, 1, n);
#else
	memset(b, 0, n*sizeof(float));
#endif
}

#endif // _TANN_UTILS_H_
