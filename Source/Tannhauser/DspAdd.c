#include "DspAdd.h"
#include "TannBase.h"

void dAdd_init(DspAdd *o, float k) {
	o->k = k;
	mq_init(&o->mq);
}

void dAdd_free(DspAdd *o) {
	mq_free(&o->mq);
}

void dAdd_processM(void *_c, DspAdd *o, float *bi0, float *bo0, const int n) {
	int z = 0;
	int x = 0;
	while (mq_hasMessage(&o->mq)) {
		PdMessage *m = mq_peek(&o->mq)->m; // get the message
		z = ctx_getBlockIndexForTimestamp(Base(_c), m->timestamp);
		dAdd_processSC(bi0+x, o->k, bo0+x, z-x);
		o->k = msg_getFloat(m, 0);
		mq_pop(&o->mq); // pop it from the queue (and free the message)
		x = z;
	}
	dAdd_processSC(bi0+z, o->k, bo0+z, n-z);
}

void dAdd_onMessage(DspAdd *o, int inletIndex, PdMessage *m) {
	// message is appended to the queue and put onto the stack (i.e. a copy is made)
	if (msg_isFloat(m, 0)) {
		mq_addMessage(&o->mq, m, inletIndex, NULL);
	}
}
