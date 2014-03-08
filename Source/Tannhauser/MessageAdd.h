#ifndef _MESSAGE_ADD_H_
#define _MESSAGE_ADD_H_

struct TannBase;
struct PdMessage;

typedef struct MessageAdd {
	float last;
	float k;
} MessageAdd;

void mAdd_init(MessageAdd *o, float k);

void mAdd_onMessage(struct TannBase *_c, MessageAdd *o, int inletIndex, struct PdMessage *message,
        void (*sendMessage)(struct TannBase *, int, struct PdMessage *));

#endif // _MESSAGE_ADD_H_
