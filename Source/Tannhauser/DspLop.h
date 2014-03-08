#ifndef _DSP_LOP_H_
#define _DSP_LOP_H_

#include "MessageQueue.h"
#if __APPLE__
	#include <Accelerate/Accelerate.h>
#endif

struct PdMessage;
struct TannBase;

typedef struct DspLop {
	float y; // the last output value (filter state)
	float b[5]; // filter weights
	MessageQueue mq;
} DspLop;

void dLop_init(struct TannBase *_c, DspLop *o, float fc, const double r);

void dLop_free(DspLop *o);

// process with only a signal input on the left inlet
void dLop_processS(DspLop *o, float *bIn, float *bOut, const int n);

// process with a signal input on the left inlet,
// and a message input at the right inlet
void dLop_processSM(struct TannBase *_c, DspLop *o, float *bIn, float *bOut, const int n, const double r);

// this funtion is only relevant for the right inlet
void dLop_onMessage(DspLop *o, int inletIndex, struct PdMessage *m);

#endif // _DSP_LOP_H_
