#ifndef _MESSAGE_QUEUE_H_
#define _MESSAGE_QUEUE_H_

#include <stdbool.h>
#include <stdlib.h>
#include "PdMessage.h"

struct TannBase;

typedef struct MessageNode {
	struct MessageNode *prev; // doubly linked list
	struct MessageNode *next;
	int let;
	PdMessage *m;
	void (*sendMessage)(struct TannBase *, int, PdMessage *);
} MessageNode;

/** A doubly linked list containing scheduled messages. */
typedef struct MessageQueue {
	MessageNode *head; // the head of the queue
	MessageNode *tail; // the tail of the queue
	MessageNode *pool; // the head of the reserve pool
} MessageQueue;

void mq_init(MessageQueue *q);

void mq_free(MessageQueue *q);

int mq_size(MessageQueue *q);

static inline PdMessage *mq_node_getMessage(MessageNode *n) {
	return n->m;
}

static inline int mq_node_getLet(MessageNode *n) {
	return n->let;
}

static inline bool mq_hasMessage(MessageQueue *q) {
	return (q->head != NULL);
}

// true if there is a message and it occurs before (<) timestamp
static inline bool mq_hasMessageBefore(MessageQueue *const q, const double timestamp) {
	return mq_hasMessage(q) && (msg_getTimestamp(mq_node_getMessage(q->head)) < timestamp);
}

static inline MessageNode *mq_peek(MessageQueue *q) {
	return q->head;
}

/** Appends the message to the end of the queue. */
PdMessage *mq_addMessage(MessageQueue *q, PdMessage *m, int let,
		void (*sendMessage)(struct TannBase *, int, PdMessage *));

/** Insert in ascending order the message acccording to its timestamp. */
PdMessage *mq_addMessageByTimestamp(MessageQueue *q, PdMessage *m, int let,
		void (*sendMessage)(struct TannBase *, int, PdMessage *));

/** Pop the message at the head of the queue (and free its memory). */
void mq_pop(MessageQueue *q);

/** Remove a message from the queue (and free its memory) */
void mq_removeMessage(MessageQueue *q, PdMessage *m);

/** Clears (and frees) all messages in the queue. */
void mq_clear(MessageQueue *q);

/** Removes all messages occuring at or after the given timestamp. */
void mq_clearAfter(MessageQueue *q, const double timestamp);

#endif // _MESSAGE_QUEUE_H_
