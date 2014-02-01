#include "DspOscillator.h"
#include "TannBase.h"
#include "sramalloc.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int hzToStep(float f, double r) {
	return (int) (4294967296.0f * f / r);
}

static float *wave = NULL; // the global oscillator wavetable
static unsigned int retainCount = 0; // the global wavetable retain count

void dOsc_init(DspOsc *o, float f, double r) {
	o->s = hzToStep(f, r);
	o->t = 0;
	mq_init(&o->mq);

	if (retainCount++ == 0) {
		// allocate the wavetable
		wave = (float *) myalloc(65536 * sizeof(float));

		// fill the wavetable with one period of a cosine
		for (int i = 0; i < 65536; i++) {
			wave[i] = cosf(6.28318530717959f * i / 65536.0f);
		}
	}
}

void dOsc_free(DspOsc *o) {
	mq_free(&o->mq);

	// free the global wavetable if there are no more references to it
	if (--retainCount == 0) {
		myfree(wave);
		wave = NULL;
	}
}

static void dOsc_processK(DspOsc *const o, float *const bOut, const int n) {
	for (int i = 0; i < n; ++i, o->t += o->s) {
		bOut[i] = wave[(unsigned short) (o->t >> 16)];
	}
  /* for (int i = 0; i < n; i++, o->t += o->s) { */
  /*   bOut[i] = sinf(2 * M_PI * ((o->t >> 16) / 65536.0f)); */
  /* } */
}

void dOsc_processM(TannBase *_c, DspOsc *o, float *bOut, const double r, const int n) {
	int z = 0;
	int x = 0;
	while (mq_hasMessage(&o->mq)) {
		MessageNode *n = mq_peek(&o->mq);
		z = ctx_getBlockIndexForTimestamp(Base(_c), msg_getTimestamp(mq_node_getMessage(n)));
		dOsc_processK(o, bOut+x, z-x);
		switch (mq_node_getLet(n)) {
			case 0: o->s = hzToStep(msg_getFloat(mq_node_getMessage(n),0), r); break;
			case 1: o->t = (unsigned short) (((unsigned int) (65536.0f * msg_getFloat(mq_node_getMessage(n),0))) % 65536); break;
			default: break;
		}
		mq_pop(&o->mq); // pop it from the queue (and free the message)
		x = z;
	}
	dOsc_processK(o, bOut+z, n-x);
}

void dOsc_processS(DspOsc *o, float *bIn, float *bOut, const double r, const int n) {
	for (int i = 0; i < n; ++i) {
		o->t += hzToStep(bIn[i], r);
		bOut[i] = wave[(unsigned short) (o->t >> 16)];
	}
}

void dOsc_onMessage(DspOsc *o, int inletIndex, PdMessage *m) {
	if (msg_isFloat(m,0)) {
		mq_addMessage(&o->mq, m, inletIndex, NULL);
	}
}
