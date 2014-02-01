#ifndef _DSP_OSCILLATOR_H_
#define _DSP_OSCILLATOR_H_

#include "MessageQueue.h"

struct TannBase;
struct PdMessage;

typedef struct DspOsc {
	int s; // based on the frequency (Hz) (step size may be negative)
	unsigned int t; // phase (theta) is an index into the osc lookup table
	MessageQueue mq;
} DspOsc;

void dOsc_init(DspOsc *o, float f, double r);

void dOsc_free(DspOsc *o);

void dOsc_processM(struct TannBase *_c, DspOsc *o, float *bOut, const double r, const int n);

void dOsc_processS(DspOsc *o, float *bIn, float *bOut, const double r, const int n);

void dOsc_onMessage(DspOsc *o, int inletIndex, struct PdMessage *m);

#endif // _DSP_OSCILLATOR_H_
