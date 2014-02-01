#include "MessageQueue.h"
#include "PdMessage.h"

void mq_init(MessageQueue *q) {
	q->head = NULL;
	q->tail = NULL;
	q->pool = NULL;
}

void mq_free(MessageQueue *q) {
	mq_clear(q);
	while (q->pool != NULL) {
		MessageNode *n = q->pool;
		q->pool = q->pool->next;
		free(n);
	}
}

static MessageNode *mq_getOrCreateNodeFromPool(MessageQueue *q) {
	if (q->pool == NULL) {
		// if necessary, create a new empty node
		q->pool = (MessageNode *) calloc(1, sizeof(MessageNode));
	}
	MessageNode *node = q->pool;
	q->pool = q->pool->next;
	return node;
}

int mq_size(MessageQueue *q) {
    int size = 0;
    MessageNode *n = q->head;
    while (n != NULL) {
        ++size;
        n = n->next;
    }
    return size;
}

PdMessage *mq_addMessage(MessageQueue *q, PdMessage *m, int let,
		void (*sendMessage)(struct TannBase *, int, PdMessage *)) {
	MessageNode *node = mq_getOrCreateNodeFromPool(q);
	node->m = msg_copy(m);
	node->let = let;
	node->sendMessage = sendMessage;
	node->prev = NULL;
	node->next = NULL;
	
	if (q->tail != NULL) {
		// the list already contains elements
		q->tail->next = node;
		node->prev = q->tail;
		q->tail = node;
	} else {
		// the list is empty
		node->prev = NULL;
		q->head = node;
		q->tail = node;
	}
	return mq_node_getMessage(node);
}

PdMessage *mq_addMessageByTimestamp(MessageQueue *q, PdMessage *m, int let,
		void (*sendMessage)(struct TannBase *, int, PdMessage *)) {
	if (mq_hasMessage(q)) {
		MessageNode *n = mq_getOrCreateNodeFromPool(q);
		n->m = msg_copy(m);
		n->let = let;
		n->sendMessage = sendMessage;
		
		if (m->timestamp < msg_getTimestamp(q->head->m)) {
			// the message occurs before the current head
			n->next = q->head;
			q->head->prev = n;
			n->prev = NULL;
			q->head = n;
		} else if (m->timestamp >= msg_getTimestamp(q->tail->m)) {
			// the message occurs after the current tail
			n->next = NULL;
			n->prev = q->tail;
			q->tail->next = n;
			q->tail = n;
		} else {
			// the message occurs somewhere between the head and tail
			MessageNode *node = q->head;
			while (node != NULL) {
				if (m->timestamp < msg_getTimestamp(node->next->m)) {
					MessageNode *r = node->next;
					node->next = n;
					n->next = r;
					n->prev = node;
					r->prev = n;
					break;
				}
				node = node->next;
			}
		}
		return n->m;
	} else {
		// add a message to the head
		return mq_addMessage(q, m, let, sendMessage);
	}
}

void mq_pop(MessageQueue *q) {
	if (mq_hasMessage(q)) {
		MessageNode *n = q->head;
		
		msg_free(n->m);
		n->m = NULL;

		n->let = 0;
		n->sendMessage = NULL;

		q->head = n->next;
		if (q->head == NULL) {
			q->tail = NULL;
		} else {
			q->head->prev = NULL;
		}
		n->next = q->pool;
		n->prev = NULL;
		q->pool = n;
	}
}

void mq_removeMessage(MessageQueue *q, PdMessage *m) {
	if (mq_hasMessage(q)) {
		if (mq_node_getMessage(q->head) == m) { // msg in head node
			mq_pop(q);
		} else {
			MessageNode *prevNode = q->head;
			MessageNode *currNode = q->head->next;
			while ((currNode != NULL) && (currNode->m != m)) {
				prevNode = currNode;
				currNode = currNode->next;
			}
			if (currNode != NULL) {
				msg_free(m);
				currNode->m = NULL;
				currNode->let = 0;
				currNode->sendMessage = NULL;
				if (currNode == q->tail) { // msg in tail node
					prevNode->next = NULL;
					q->tail = prevNode;
				} else { // msg in middle node
					prevNode->next = currNode->next;
					currNode->next->prev = prevNode;
				}
				currNode->next = (q->pool == NULL) ? NULL : q->pool;
				currNode->prev = NULL;
				q->pool = currNode;
			}
		}
	}
}

void mq_clear(MessageQueue *q) {
	while (mq_hasMessage(q)) {
		mq_pop(q);
	}
}

void mq_clearAfter(MessageQueue *q, const double timestamp) {
	MessageNode *n = q->tail;
	while (n != NULL && timestamp <= msg_getTimestamp(n->m)) {
		// free the node's message
		msg_free(n->m);
		n->m = NULL;
		n->let = 0;
		n->sendMessage = NULL;
		
		// the tail points at the previous node
		q->tail = n->prev;
		
		// put the node back in the pool
		n->next = q->pool;
		n->prev = NULL;
		if (q->pool != NULL) q->pool->prev = n;
		q->pool = n;
		
		// update the tail node
		n = q->tail;
	}

	if (q->tail == NULL) q->head = NULL;
}
