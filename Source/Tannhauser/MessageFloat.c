#include "MessageFloat.h"
#include "TannBase.h"

void mFloat_init(MessageFloat *o, float k) {
	o->k = k;
}

void mFloat_onMessage(TannBase *_c, MessageFloat *o, int inletIndex, PdMessage *m,
        void (*sendMessage)(TannBase *, int, PdMessage *)) {
	switch (inletIndex) {
		case 0: {
			switch (msg_getType(m, 0)) {
				case FLOAT: {
					o->k = msg_getFloat(m, 0);
                    // allow fallthrough
				}
                case BANG: {
                    PdMessage *n = PD_MESSAGE_ON_STACK(1);
                    msg_initWithFloat(n, m->timestamp, o->k);
                    sendMessage(_c, 0, n);
                    break;
                }
				default: break;
			}
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
