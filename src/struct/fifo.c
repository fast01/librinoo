/**
 * @file   fifo.c
 * @author Reginald LIPS <reginald.l@gmail.com> - Copyright 2012
 * @date   Fri Jan 29 15:00:41 2010
 *
 * @brief  Functions to manage fifo list structures.
 *
 *
 */

#include "rinoo/rinoo.h"

/**
 * Creates a new fifo list.
 *
 *
 * @return Pointer to the new fifo list, or NULL if an error occurs.
 */
t_fifo *fifo_create()
{
	t_fifo *fifo;

	fifo = calloc(1, sizeof(*fifo));
	if (unlikely(fifo == NULL)) {
		return NULL;
	}
	return fifo;
}

/**
 * Destroys a fifo list and its elements.
 *
 * @param ptr
 */
void fifo_destroy(void *ptr)
{
	t_fifo *fifo;
	t_fifonode *cur;
	t_fifonode *tmp;

	XASSERTN(ptr != NULL);

	fifo = ptr;
	cur = fifo->head;
	while (cur != NULL) {
		tmp = cur->next;
		if (cur->free_func != NULL) {
			cur->free_func(cur->node);
		}
		free(cur);
		cur = tmp;
	}
	free(fifo);
}

/**
 * Push an element into the fifo list.
 *
 * @param fifo Pointer to the fifo list to use.
 * @param node Pointer to the element to add.
 * @param free_func Pointer to a function which can free the element.
 *
 * @return 0 on success, -1 if an error occurs.
 */
int fifo_push(t_fifo *fifo, void *node, void (*free_func)(void *node))
{
	t_fifonode *new;

	XASSERT(fifo != NULL, -1);
	XASSERT(node != NULL, -1);

	new = calloc(1, sizeof(*new));
	if (unlikely(new == NULL)) {
		return -1;
	}
	new->node = node;
	new->free_func = free_func;
	if (fifo->head == NULL) {
		fifo->head = new;
	}
	if (fifo->tail != NULL) {
		fifo->tail->next = new;
	}
	fifo->tail = new;
	fifo->size++;
	return 0;
}

/**
 * Get the first element of the fifo. It will be removed from the list.
 *
 * @param fifo Pointer to the fifo list to use.
 *
 * @return Pointer to the element.
 */
void *fifo_pop(t_fifo *fifo)
{
	t_fifonode *elem;
	void *node;

	XASSERT(fifo != NULL, NULL);

	if (fifo->head == NULL) {
		return NULL;
	}
	elem = fifo->head;
	fifo->head = elem->next;
	if (fifo->head == NULL) {
		fifo->tail = NULL;
	}
	node = elem->node;
	free(elem);
	fifo->size--;
	return node;
}

/**
 * Get the first element of the fifo. The node is still in the fifo.
 *
 * @param fifo Pointer to the fifo list to use.
 *
 * @return Pointer to the element.
 */
void *fifo_get(t_fifo *fifo)
{
	XASSERT(fifo != NULL, NULL);

	if (fifo->head == NULL) {
		return NULL;
	}
	return fifo->head->node;
}
