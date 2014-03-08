#ifndef _DSP_HIP_H_
#define _DSP_HIP_H_

#include "MessageQueue.h"

struct TannBase;
struct PdMessage;

typedef struct DspHip {
	float x; // the last output values (filter state)
	float y;
	float b[5]; // filter weights
	MessageQueue mq;
} DspHip;

void dHip_init(struct TannBase *_c, DspHip *o, float fc, const double r);

void dHip_free(DspHip *o);

void dHip_processS(DspHip *o, float *bIn, float *bOut, const int n);

void dHip_processSM(struct TannBase *_c, DspHip *o, float *bIn, float *bOut, const int n, const double r);

void dHip_onMessage(DspHip *o, int inletIndex, struct PdMessage *m);

#endif // _DSP_HIP_H_
