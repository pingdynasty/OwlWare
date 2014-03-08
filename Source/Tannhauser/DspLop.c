#define _USE_MATH_DEFINES
#include <math.h>
#include "DspLop.h"
#include "TannBase.h"

static void dLop_updateFilterCoeff(void *_c, DspLop *o, float fc, const double r) {
	// 0 <= fx < SAMPLE_RATE/2
	fc = fmaxf(0.0f, fminf(fc, (float) (0.5*r)));

	const double wc = 2.0 * M_PI * fc;
	const float alpha = (float) (wc / (wc + r));
	o->b[0] = alpha;
	o->b[1] = 0.0f;
	o->b[2] = 0.0f;
	o->b[3] = -(1.0f-alpha);
	o->b[4] = 0.0f;
}

void dLop_init(TannBase *_c, DspLop *o, float fc, const double r) {
	mq_init(&o->mq);
	dLop_updateFilterCoeff(_c, o, fc, r);
}

void dLop_free(DspLop *o) {
	mq_free(&o->mq);
}

void dLop_processS(DspLop *o, float *bIn, float *bOut, const int n) {
#if __APPLE__
	const int nn = n + 2;
	float tIn[nn];
	tIn[0] = 0.0f;
	tIn[1] = 0.0f;
	memcpy(tIn+2, bIn, sizeof(float)*n);
	
	float tOut[nn];
	tOut[0] = 0.0f;
	tOut[1] = o->y;
	
	vDSP_deq22(tIn, 1, o->b, tOut, 1, n);
	
	o->y = tOut[n+1];
	memcpy(bOut, tOut+2, sizeof(float)*n);
#else
	float y = o->y;
	const float a = o->b[0];
	const float b = o->b[3];
	for (int i = 0; i < n; ++i) {
		y = (a * bIn[i]) - (b * y);
		bOut[i] = y;
	}
	o->y = y;
#endif
}

void dLop_processSM(TannBase *_c, DspLop *o, float *bIn, float *bOut, const int n, const double r) {
	int z = 0;
	int x = 0;
	while (mq_hasMessage(&o->mq)) {
		PdMessage *m = mq_peek(&o->mq)->m;
		z = ctx_getBlockIndexForTimestamp(_c, msg_getTimestamp(m));
		dLop_processS(o, bIn+x, bOut+x, z-x);
		dLop_updateFilterCoeff(_c, o, msg_getFloat(m, 0), r);
		mq_pop(&o->mq);
		x = z;
	}
	dLop_processS(o, bIn+z, bOut+z, n-z);
}

void dLop_onMessage(DspLop *o, int inletIndex, PdMessage *m) {
	if (msg_isFloat(m, 0)) {
		mq_addMessage(&o->mq, m, inletIndex, NULL);
	}
}
