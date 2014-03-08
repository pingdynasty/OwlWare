#include "MessageLine.h"
#include "TannBase.h"

void mLine_init(MessageLine *o, float v, double r) {
	o->v = v;
	o->m = 0.0f;
	o->t = v;
	o->r = r;
	o->u = 0.0;
	o->n = NULL;
}

void mLine_onMessage(TannBase *_c, MessageLine *o, int inletIndex, PdMessage *m,
		void (*sendMessage)(TannBase *, int, PdMessage *)) {
	switch (inletIndex) {
		case 0: {
			if (msg_hasFormat(m, "f")) {
				// cancel any previously scheduled message
				ctx_cancelMessage(Base(_c), o->n);
				o->n = NULL;
				
				// immediately go to the specified value
				o->v = msg_getFloat(m,0);
				o->m = 0.0f;
				o->t = msg_getFloat(m,0);
				o->u = msg_getTimestamp(m);
				
				PdMessage *n = PD_MESSAGE_ON_STACK(1);
				msg_initWithFloat(n, msg_getTimestamp(m), o->v);
				sendMessage(_c, 0, n);
			} else if (msg_hasFormat(m, "ff")) {
				// cancel any previously scheduled message
				ctx_cancelMessage(Base(_c), o->n);
				o->n = NULL;
				
				// update current value based on receive time of this message
				o->v += (o->m * ((float) (msg_getTimestamp(m) - o->u)));

				o->t = msg_getFloat(m,0); // update target
				o->m = (o->t - o->v) / msg_getFloat(m,1); // slope (units/millisecond)
				o->u = msg_getTimestamp(m);
				
				PdMessage *n = PD_MESSAGE_ON_STACK(1);
				msg_initWithFloat(n, msg_getTimestamp(m), o->v);
				sendMessage(_c, 0, n);
			} else if (msg_compareSymbol(m, 0, "stop")) {
				// cancel any previously scheduled message
				ctx_cancelMessage(Base(_c), o->n);
				o->n = NULL;
				
				o->v += (o->m * ((float) (msg_getTimestamp(m) - o->u))); // update current value
				o->m = 0.0f;
				o->t = o->v;
				o->u = msg_getTimestamp(m);
			} else if (msg_hasFormat(m, "sf") && msg_compareSymbol(m, 0, "set")) {
				o->v = msg_getFloat(m,1);
				o->m = 0.0f;
				o->t = o->v;
				o->u = msg_getTimestamp(m);
				// don't send any message
			}
			break;
		}
		case 1: {
			if (msg_isFloat(m,0)) {
				o->r = msg_getFloat(m,0); // update the grain rate
			}
			break;
		}
		default: break;
	}
}
