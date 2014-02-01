#ifndef _MESSAGE_MULTIPLY_H_
#define _MESSAGE_MULTIPLY_H_

struct TannBase;
struct PdMessage;

typedef struct MessageMultiply {
	float last;
	float k;
} MessageMultiply;

void mMultiply_init(MessageMultiply *o, float k);

void mMultiply_onMessage(struct TannBase *_c, MessageMultiply *o, int inletIndex, struct PdMessage *message,
        void (*sendMessage)(struct TannBase *, int, struct PdMessage *));

#endif // _MESSAGE_MULTIPLY_H_
