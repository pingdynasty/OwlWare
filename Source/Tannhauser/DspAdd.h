#ifndef _DSP_ADD_H_
#define _DSP_ADD_H_

#include "MessageQueue.h"
#if __APPLE__
	#include <Accelerate/Accelerate.h>
#endif

struct PdMessage;

typedef struct DspAdd {
	float k;
	MessageQueue mq;
} DspAdd;

void dAdd_init(DspAdd *o, float k);

void dAdd_free(DspAdd *o);

static inline void dAdd_processSS(float *bi0, float *bi1, float *bo0, const int n) {
	#if __APPLE__
		vDSP_vadd(bi0, 1, bi1, 1, bo0, 1, n);
	#else
		for (int i = 0; i < n; ++i) {
			bo0[i] = bi0[i] + bi1[i];
		}
	#endif
}

static inline void dAdd_processSC(float *bi0, float k, float *bo0, const int n) {
	#if __APPLE__
		vDSP_vsadd(bi0, 1, &k, bo0, 1, n);
	#else
		for (int i = 0; i < n; ++i) {
			bo0[i] = bi0[i] + k;
		}
	#endif
}

void dAdd_processM(void *_c, DspAdd *o, float *bi0, float *bo0, const int n);

void dAdd_onMessage(DspAdd *o, int inletIndex, struct PdMessage *m);

#endif // _DSP_ADD_H_
