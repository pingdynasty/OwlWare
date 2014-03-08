#define _USE_MATH_DEFINES
#include <math.h>
#include "DspHip.h"
#include "TannBase.h"
#if __APPLE__
#include <Accelerate/Accelerate.h>
#endif

static void dHip_updateFilterCoeff(TannBase *_c, DspHip *o, float fc, const double r) {
	// 0 <= fx < SAMPLE_RATE/2
	fc = fmaxf(0.0f, fminf(fc, (float) (0.5*r)));
	
	float alpha = (float) (r / ((2.0*M_PI*fc) + r));
	o->b[0] = alpha;
	o->b[1] = -alpha;
	o->b[2] = 0.0f;
	o->b[3] = -alpha;
	o->b[4] = 0.0f;
}

void dHip_init(TannBase *_c, DspHip *o, float fc, const double r) {
	o->x = 0.0f; o->y = 0.0f;
	mq_init(&o->mq);
	dHip_updateFilterCoeff(_c, o, fc, r);
}

void dHip_free(DspHip *o) {
	mq_free(&o->mq);
}

void dHip_processS(DspHip *o, float *bIn, float *bOut, const int n) {
	const int nn = n + 2;
	float tIn[nn]; tIn[0] = 0.0f; tIn[1] = o->x;
	memcpy(tIn+2, bIn, n*sizeof(float));
	
	float tOut[nn]; tOut[0] = 0.0f; tOut[1] = o->y;
	
#if __APPLE__
	vDSP_deq22(tIn, 1, o->b, tOut, 1, n);
#else
	for (int i = 2; i < nn; ++i) {
		tOut[i] = (o->b[0] * tIn[i]) + (o->b[1] * tIn[i-1]) - (o->b[3] * tOut[i-1]);
	}
#endif
	o->x = tIn[n+1]; o->y = tOut[n+1];
	memcpy(bOut, tOut+2, n*sizeof(float));
}

void dHip_processSM(TannBase *_c, DspHip *o, float *bIn, float *bOut, const int n, const double r) {
	int z = 0;
	int x = 0;
	while (mq_hasMessage(&o->mq)) {
		PdMessage *m = mq_peek(&o->mq)->m;
		z = ctx_getBlockIndexForTimestamp((TannBase *) _c, m->timestamp);
		dHip_processS(o, bIn+x, bOut+x, z-x);
		dHip_updateFilterCoeff(_c, o, msg_getFloat(m, 0), r);
		mq_pop(&o->mq);
		x = z;
	}
	dHip_processS(o, bIn+z, bOut+z, n-z);
}

void dHip_onMessage(DspHip *o, int inletIndex, PdMessage *m) {
	if (msg_isFloat(m, 0)) {
		mq_addMessage(&o->mq, m, inletIndex, NULL);
	}
}
