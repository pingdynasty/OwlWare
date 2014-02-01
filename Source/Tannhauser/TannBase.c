#include <stdarg.h>
#include "TannBase.h"

void ctx_setBasePath(TannBase *const _c, const char *basePath) {
	myfree(_c->basePath);
	size_t len = strlen(basePath);
	_c->basePath = (char *) myalloc((len+1)*sizeof(char));
	strcpy(_c->basePath, basePath);
}

void ctx_cancelMessage(TannBase *_c, PdMessage *m) {
	mq_removeMessage(&_c->mq, m);
}

void ctx_scheduleMessageForReceiverV(TannBase *const _c, const char *name, const double timestamp, const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	
	const int numElem = (int) strlen(format);
	PdMessage *m = PD_MESSAGE_ON_STACK(numElem);
	msg_init(m, numElem, timestamp);
	for (int i = 0; i < numElem; i++) {
		switch (format[i]) {
			case 'b': {
				msg_setBang(m,i);
				break;
			}
			case 'f': {
				msg_setFloat(m, i, (float) va_arg(ap, double));
				break;
			}
			case 's': {
				msg_setSymbol(m, i, (char *) va_arg(ap, char *));
				break;
			}
			default: break;
		}
	}
	_c->f_scheduleMessageForReceiver(_c, name, m);
	
	va_end(ap);
}
