#include <stdarg.h>
#include <stdio.h>
#include "PdMessage.h"
#include "sramalloc.h"

void msg_init(PdMessage *m, int numElements, double timestamp) {
    m->numElements = numElements;
    m->timestamp = timestamp;
}

void msg_initWithFloat(PdMessage *m, double timestamp, float f) {
    m->numElements = 1;
    m->timestamp = timestamp;
    msg_setFloat(m, 0, f);
}

void msg_initWithBang(PdMessage *m, double timestamp) {
    m->numElements = 1;
    m->timestamp = timestamp;
    msg_setBang(m, 0);
}

void msg_initWithSymbol(PdMessage *m, double timestamp, char *s) {
    m->numElements = 1;
    m->timestamp = timestamp;
    msg_setSymbol(m, 0, s);
}

PdMessage *msg_initV(PdMessage *const m, const double timestamp, const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	
	const int numElem = (int) strlen(format);
	msg_init(m, numElem, timestamp);
	for (int i = 0; i < numElem; i++) {
		switch (format[i]) {
			case 'b': msg_setBang(m,i); break;
			case 'f': msg_setFloat(m, i, (float) va_arg(ap, double)); break;
			case 's': msg_setSymbol(m, i, (char *) va_arg(ap, char *)); break;
			default: break;
		}
	}
	va_end(ap);
	
	return m;
}

/** Returns the total length in bytes of this message for a given number of elements. */
int msg_getByteSize(int numElements) {
    return (numElements > 1)
            ? sizeof(PdMessage) + (numElements-1) * sizeof(Element)
            : sizeof(PdMessage);
}

// the message is serialised such that all symbol elements are placed in order at the end of the buffer
PdMessage *msg_copy(PdMessage *m) {

	// calculate the basic size of message
	const int rsizeof = msg_getByteSize(msg_getNumElements(m));

	// calculate the size of the symbol elements
	int slen[msg_getNumElements(m)]; // the byte length of each symbol
	int rsizeofsym = 0;
	for (int i = 0; i < msg_getNumElements(m); ++i) {
		if (msg_isSymbol(m,i)) {
			slen[i] = (int) strlen(msg_getSymbol(m,i)) + 1; // +1 to allow for trailing '\0'
			rsizeofsym += slen[i];
		} else {
			slen[i] = 0;
		}
	}
	PdMessage *r = (PdMessage *) myalloc(rsizeof + rsizeofsym);
	
	// copy to the original message to the first part of the buffer
	memcpy(r, m, rsizeof);
	
	if (rsizeofsym > 0) { // if there are any symbol elements at all
		char *p = ((char *) r) + rsizeof; // points to the end of the base message
		for (int i = 0; i < msg_getNumElements(m); ++i) {
			if (msg_isSymbol(m,i)) {
				memcpy(p, msg_getSymbol(m,i), slen[i]);
				msg_setSymbol(r, i, p);
				p += slen[i];
			}
		}
	}

    return r;
}

void msg_free(PdMessage *m) {
    myfree(m); // because heap messages are serialised, a simple call to free releases the message
}

int msg_hasFormat(PdMessage *m, const char *fmt) {
	if (fmt == NULL) return 0;
	if (m->numElements != strlen(fmt)) return 0;
	for (int i = 0; i < m->numElements; i++) {
		switch (fmt[i]) {
			case 'b': if (!msg_isBang(m, i)) return 0; break;
			case 'f': if (!msg_isFloat(m, i)) return 0; break;
			case 's': if (!msg_isSymbol(m, i)) return 0; break;
			default: return 0;
		}
	}
	return 1;
}

int msg_equalsElement(PdMessage *m, int i_m, PdMessage *n, int i_n) {
	if (i_m < msg_getNumElements(m) && i_n < msg_getNumElements(n)) {
		if (msg_getType(m, i_m) == msg_getType(n, i_n)) {
			switch (msg_getType(m, i_m)) {
				case BANG: return 1;
				case FLOAT: return (msg_getFloat(m, i_m) == msg_getFloat(n, i_n));
				case SYMBOL: return msg_compareSymbol(m, i_m, msg_getSymbol(n, i_n));
				default: break;
			}
		}
	}
	return 0;
}

void msg_setElementToFrom(PdMessage *n, int i_n, PdMessage *m, int i_m) {
	// NOTE(mhroth): there is no index bounds checking here
	switch (msg_getType(m, i_m)) {
		case BANG: msg_setBang(n, i_n); break;
		case FLOAT: msg_setFloat(n, i_n, msg_getFloat(m, i_m)); break;
		case SYMBOL: msg_setSymbol(n, i_n, msg_getSymbol(m, i_m)); break;
		default: break;
	}
}

char *msg_toString(PdMessage *m) {
    // http://stackoverflow.com/questions/295013/using-sprintf-without-a-manually-allocated-buffer
    int lengths[m->numElements]; // how long is the string of each atom
    char *finalString; // the final buffer we will pass back after concatenating all strings - user should free it
    int size = 0; // the total length of our final buffer
    int pos = 0;
    
    // loop through every element in our list of atoms
    // first loop figures out how long our buffer should be
    // chrism: apparently this might fail under MSVC because of snprintf(NULL) - do we care?
    for (int i = 0; i < msg_getNumElements(m); i++) {
        lengths[i] = 0;
        switch (msg_getType(m, i)) {
            case BANG: lengths[i] = snprintf(NULL, 0, "%s", "bang"); break;
            case FLOAT: lengths[i] = snprintf(NULL, 0, "%g", msg_getFloat(m, i)); break;
            case SYMBOL:lengths[i] = snprintf(NULL, 0, "%s", msg_getSymbol(m, i)); break;
            default: break;
        }
        // total length of our string is each atom plus a space, or \0 on the end
        size += lengths[i] + 1;
    }
    
    // now we do the piecewise concatenation into our final string
    /* finalString = (char *) _mm_malloc(size <= 0 ? 1 : size, sizeof(char)); // ensure that size is at least 1 */
    int len = (size <= 0 ? 1 : size) * sizeof(char);
    finalString = (char *) myalloc(len); // ensure that size is at least 1
    memset(finalString, 0, len);
    for (int i = 0; i < msg_getNumElements(m); i++) {
        // first element doesn't have a space before it
        if (i > 0) {
            strncat(finalString, " ", 1);
            pos += 1;
        }
        // put a string representation of each atom into the final string
        switch (msg_getType(m, i)) {
            case BANG: snprintf(&finalString[pos], lengths[i] + 1, "%s", "bang"); break;
            case FLOAT: snprintf(&finalString[pos], lengths[i] + 1, "%g", msg_getFloat(m, i)); break;
            case SYMBOL: snprintf(&finalString[pos], lengths[i] + 1, "%s", msg_getSymbol(m, i)); break;
            default: break;
        }
        pos += lengths[i];
    }
    return finalString;

}
