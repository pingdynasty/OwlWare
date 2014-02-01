#include "DspMultiply.h"
#include "TannBase.h"

void dMult_init(DspMultiply *o, float k) {
	o->k = k;
	mq_init(&o->mq);
}

void dMult_free(DspMultiply *o) {
	mq_free(&o->mq);
}

void dMult_processM(TannBase *_c, DspMultiply *o, float *bIn, float *bOut, const int n) {
	int z = 0;
	int x = 0;
	while (mq_hasMessage(&o->mq)) {
		PdMessage *m = mq_peek(&o->mq)->m; // get the message
		z = ctx_getBlockIndexForTimestamp(_c, m->timestamp);
		dMult_processSK(bIn+x, o->k, bOut+x, z-x);
		o->k = msg_getFloat(m, 0);
		mq_pop(&o->mq); // pop it from the queue (and free the message)
		x = z;
	}
	dMult_processSK(bIn+z, o->k, bOut+z, n-z);
}

void dMult_onMessage(DspMultiply *o, int inletIndex, PdMessage *m) {
	if (msg_isFloat(m, 0)) {
		mq_addMessage(&o->mq, m, inletIndex, NULL);
	}
}
