#ifndef _PD_MESSAGE_H_
#define _PD_MESSAGE_H_

//#include <stdlib.h>
#include <string.h>
#include <alloca.h>

typedef enum ElementType {
	BANG,
 	FLOAT,
 	SYMBOL
} ElementType;

typedef struct Element {
	ElementType type;
	union {
		float f;
		char *s;
	} data;
} Element;

typedef struct PdMessage {
	double timestamp;
	int numElements;
	Element elem;
} PdMessage;

#define PD_MESSAGE_ON_STACK(_x) (PdMessage *) alloca(sizeof(PdMessage) + (((_x) > 1 ? (_x)-1 : 0)*sizeof(Element)));

int msg_getByteSize(int numElements);

PdMessage *msg_copy(PdMessage *m);

void msg_setElementToFrom(PdMessage *n,  int indexN, PdMessage *m,  int indexM);

/** Frees a message on the heap. Does nothing if argument is NULL. */
void msg_free(PdMessage *m);

void msg_init(PdMessage *m, int numElements, double timestamp);

void msg_initWithFloat(PdMessage *m, double timestamp, float f);

void msg_initWithBang(PdMessage *m, double timestamp);

void msg_initWithSymbol(PdMessage *m, double timestamp, char *s);

PdMessage *msg_initV(PdMessage *const m, const double timestamp, const char *format, ...);

static inline double msg_getTimestamp(PdMessage *m) {
	return m->timestamp;
}

static inline void msg_setTimestamp(PdMessage *m, double timestamp) {
	m->timestamp = timestamp;
}

static inline int msg_getNumElements(PdMessage *m) {
	return m->numElements;
}

static inline ElementType msg_getType(PdMessage *m, int index) {
	return (&(m->elem)+index)->type;
}

static inline void msg_setBang(PdMessage *m, int index) {
	(&(m->elem)+index)->type = BANG;
	(&(m->elem)+index)->data.f = 0.0f;
}

static inline int msg_isBang(PdMessage *m, int index) {
	return (&(m->elem)+index)->type == BANG;
}

static inline float msg_getFloat(PdMessage *m, int index) {
	return (&(m->elem)+index)->data.f;
}

static inline void msg_setFloat(PdMessage *m, int index, float f) {
	(&(m->elem)+index)->type = FLOAT;
	(&(m->elem)+index)->data.f = f;
}

static inline int msg_isFloat(PdMessage *m, int index) {
	return (&(m->elem)+index)->type == FLOAT;
}

static inline void msg_setSymbol(PdMessage *m, int index, char *s) {
	(&(m->elem)+index)->type = SYMBOL;
	(&(m->elem)+index)->data.s = s;
}

static inline char *msg_getSymbol(PdMessage *m, int index) {
	return (&(m->elem)+index)->data.s;
}

static inline int msg_isSymbol(PdMessage *m, int index) {
	return (&(m->elem)+index)->type == SYMBOL;
}

static inline int msg_compareSymbol(PdMessage *m, int index, const char *symbol) {
	if (!msg_isSymbol(m, index)) return 0;
	return !strcmp(msg_getSymbol(m, index), symbol);
}

/** Returns 1 if the element i_m of message m is equal to element i_n of message n. */
int msg_equalsElement(PdMessage *m, int i_m, PdMessage *n, int i_n);

int msg_hasFormat(PdMessage *m, const char *fmt);

/**
 * Create a string representation of the message. Suitable for use by the print object.
 * The resulting string must be freed by the caller.
 */
char *msg_toString(PdMessage *msg);

#endif // _PD_MESSAGE_H_
