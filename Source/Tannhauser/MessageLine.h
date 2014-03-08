#ifndef _MESSAGE_LINE_H_
#define _MESSAGE_LINE_H_

struct TannBase;
struct PdMessage;

typedef struct MessageLine {
	float v; // current value
	float m; // slope
	float t; // target value
	double r; // grain (message) interval
	double u; // the timestamp of the most recent sent message
	struct PdMessage *n; // next scheduled message
} MessageLine;

void mLine_init(MessageLine *o, float v, double r);

void mLine_onMessage(struct TannBase *_c, MessageLine *o, int inletIndex, struct PdMessage *m,
		void (*sendMessage)(struct TannBase *, int, struct PdMessage *));

#endif // _MESSAGE_LINE_H_
