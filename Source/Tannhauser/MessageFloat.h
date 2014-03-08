#ifndef _MESSAGE_FLOAT_H_
#define _MESSAGE_FLOAT_H_

struct TannBase;
struct PdMessage;

typedef struct MessageFloat {
	float k;
} MessageFloat;

void mFloat_init(MessageFloat *o, float k);

void mFloat_onMessage(struct TannBase *_c, MessageFloat *o, int inletIndex, struct PdMessage *m,
        void (*sendMessage)(struct TannBase *, int, struct PdMessage *));

#endif // _MESSAGE_FLOAT_H_
