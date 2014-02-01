#ifndef _MESSAGE_TABLE_H_
#define _MESSAGE_TABLE_H_

typedef struct MessageTable {
	float *buffer;
	int length;
} MessageTable;

void mTable_init(MessageTable *o, int length);

void mTable_free(MessageTable *o);

void mTable_resize(MessageTable *o, int newLength);

static inline float *mTable_getBuffer(MessageTable *o) {
	return o->buffer;
}

static inline int mTable_getLength(MessageTable *o) {
	return o->length;
}

#endif // _MESSAGE_TABLE_H_
