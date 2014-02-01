#include "MessageMultiply.h"
#include "TannBase.h"

void mMultiply_init(MessageMultiply *o, float k) {
	o->last = 0.0f;
	o->k = k;
}

void mMultiply_onMessage(TannBase *_c, MessageMultiply *o, int inletIndex, PdMessage *m,
        void (*sendMessage)(TannBase *, int, struct PdMessage *)) {
	switch (inletIndex) {
		case 0: {
			switch (msg_getType(m, 0)) {
				case BANG: break;
				case FLOAT: {
					o->last = msg_getFloat(m, 0) * o->k;
					break;
				}
				default: return;
			}

			// create the outgoing message
			PdMessage *n = PD_MESSAGE_ON_STACK(1);
			msg_initWithFloat(n, m->timestamp, o->last);

			// send to all outlets
			sendMessage(_c, 0, n);
			break;
		}
		case 1: {
			if (msg_isFloat(m, 0)) {
				o->k = msg_getFloat(m, 0);
			}
			break;
		}
		default: break;
	}
}
