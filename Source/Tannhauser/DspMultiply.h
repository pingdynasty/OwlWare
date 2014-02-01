#ifndef _DSP_MULTIPLY_H_
#define _DSP_MULTIPLY_H_

#include "MessageQueue.h"
#if __APPLE__
#include <Accelerate/Accelerate.h>
#endif

struct TannBase;
struct PdMessage;

typedef struct DspMultiply {
	float k;
	MessageQueue mq;
} DspMultiply;

void dMult_init(DspMultiply *o, float k);

void dMult_free(DspMultiply *o);

static inline void dMult_processSS(float *bIn0, float *bIn1, float *bOut, const int n) {
#if __APPLE__
	vDSP_vmul(bIn0, 1, bIn1, 1, bOut, 1, n);
#else
	for (int i = 0; i < n; ++i) {
		bOut[i] = bIn0[i] * bIn1[i];
	}
#endif
}

static inline void dMult_processSK(float *bIn, float k, float *bOut, const int n) {
#if __APPLE__
	vDSP_vsmul(bIn, 1, &k, bOut, 1, n);
#else
	for (int i = 0; i < n; ++i) {
		bOut[i] = bIn[i] * k;
	}
#endif
}

static inline void dMult_process_negate(float *bIn, float *bOut, int n) {
#if __APPLE__
	vDSP_vneg(bIn, 1, bOut, 1, n);
#else
	for (int i = 0; i < n; ++i) {
		bOut[i] = -1.0f * bIn[i];
	}
#endif
// http://fastcpp.blogspot.com/2011/03/changing-sign-of-float-values-using-sse.html
}

void dMult_processM(struct TannBase *_c, DspMultiply *o, float *bIn, float *bOut, const int n);

void dMult_onMessage(DspMultiply *o, int inletIndex, struct PdMessage *m);

#endif // _DSP_MULTIPLY_H_
