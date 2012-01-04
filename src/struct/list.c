/**
 * @file   list.c
 * @author Reginald LIPS <reginald.l@gmail.com> - Copyright 2012
 * @date   Tue Apr 27 10:16:26 2010
 *
 * @brief  Functions to manage linked list structures.
 *
 *
 */

#include "rinoo/rinoo.h"

/**
 * Creates a new list.
 *
 * @param type Type of the list to create.
 * @param cmp_func Pointer to a compare function.
 *
 * @return A pointer to the new list, or NULL if an error occurs.
 */
t_list *list_create(t_listtype type, int (*cmp_func)(void *node1, void *node2))
{
	t_list *list;

	XASSERT(cmp_func != NULL, NULL);

	list = calloc(1, sizeof(*list));
	if (unlikely(list == NULL)) {
		return NULL;
	}
	list->type = type;
	list->cmp_func = cmp_func;
	return list;
}

/**
 * Destroys a list and its elements
 *
 * @param ptr Pointer to the list to destroy.
 */
void list_destroy(void *ptr)
{
	t_list *list;
	t_listnode *cur;
	t_listnode *tmp;

	XASSERTN(ptr != NULL);

	list = (t_list *) ptr;

	cur = list->head;
	while (cur != NULL) {
		tmp = cur->next;
		if (cur->free_func != NULL) {
			cur->free_func(cur->node);
		}
		free(cur);
		cur = tmp;
	}
	free(list);
}

/**
 * Inserts a list node between two others.
 *
 * @param list Pointer to the list.
 * @param new Pointer to the new element to insert.
 * @param prev Pointer to the previous element (NULL if head).
 * @param next Pointer to the next element (NULL if tail).
 */
void list_insertnode(t_list *list, t_listnode *new, t_listnode *prev, t_listnode *next)
{
	XASSERTN(list != NULL);
	XASSERTN(new != NULL);

	new->prev = prev;
	new->next = next;
	if (prev == NULL) {
		list->head = new;
	} else {
		prev->next = new;
	}
	if (next == NULL) {
		list->tail = new;
	} else {
		next->prev = new;
	}
	list->size++;
}

/**
 * Adds an already existing listnode to a list.
 *
 * @param list Pointer to the list.
 * @param new Pointer to the new listnode to add.
 *
 * @return 0 on success, -1 if an error occurs.
 */
int list_addnode(t_list *list, t_listnode *new)
{
	XASSERT(list != NULL, -1);
	XASSERT(new != NULL, -1);

	switch (list->type) {
	case LIST_SORTED_HEAD:
		new->next = list->head;
		new->prev = NULL;
		while (new->next != NULL &&
		       list->cmp_func(new->node, new->next->node) > 0) {
			new->prev = new->next;
			new->next = new->next->next;
		}
		break;
	case LIST_SORTED_TAIL:
		new->next = NULL;
		new->prev = list->tail;
		while (new->prev != NULL &&
		       list->cmp_func(new->node, new->prev->node) < 0) {
			new->next = new->prev;
			new->prev = new->prev->prev;
		}
		break;
	}
	list_insertnode(list, new, new->prev, new->next);
	return 0;
}

/**
 * Adds an element to a list.
 *
 * @param list Pointer to the list where to add the element.
 * @param node Pointer to the element to add.
 * @param free_func Pointer to a function which can free the element.
 *
 * @return A pointer to the new list node, or NULL if an error occurs.
 */
t_listnode *list_add(t_list *list, void *node, void (*free_func)(void *node))
{
	t_listnode *new;

	XASSERT(list != NULL, NULL);

	new = calloc(1, sizeof(*new));
	if (unlikely(new == NULL)) {
		return NULL;
	}
	new->node = node;
	new->free_func = free_func;
	if (unlikely(list_addnode(list, new) != 0)) {
		free(new);
		return NULL;
	}
	return new;
}

/**
 * Deletes a list node from a list.
 *
 * @param list Pointer to the list to use.
 * @param node Pointer to the list node to remove.
 * @param needfree Boolean which indicates if the free_func has to be called.
 *
 * @return 0 on success, -1 if an error occurs.
 */
int list_removenode(t_list *list, t_listnode *node, u32 needfree)
{
	XASSERT(list != NULL, -1);
	XASSERT(node != NULL, -1);

	if (node->prev == NULL) {
		list->head = node->next;
	} else {
		node->prev->next = node->next;
	}
	if (node->next == NULL) {
		list->tail = node->prev;
	} else {
		node->next->prev = node->prev;
	}
	if (node->free_func != NULL && needfree == TRUE) {
		node->free_func(node->node);
	}
	free(node);
	list->size--;
	return 0;
}

/**
 * Deletes an element from a list.
 *
 * @param list Pointer to the list to use.
 * @param node Pointer to the element to remove.
 * @param needfree Boolean which indicates if the free_func has to be called.
 *
 * @return 0 on success, -1 if an error occurs.
 */
int list_remove(t_list *list, void *node, u32 needfree)
{
	t_listnode *cur;

	XASSERT(list != NULL, -1);

	cur = list->head;
	while (cur != NULL && list->cmp_func(cur->node, node) != 0) {
		cur = cur->next;
	}
	if (cur != NULL) {
		list_removenode(list, cur, needfree);
		return 0;
	}
	return -1;
}

/**
 * Finds an element in a list.
 *
 * @param list Pointer to the list to use.
 * @param node Pointer to an element to look for.
 *
 * @return Pointer to the element found or NULL if nothing is found.
 */
void *list_find(t_list *list, void *node)
{
	t_listnode *cur;

	XASSERT(list != NULL, NULL);

	cur = list->head;
	while (cur != NULL && list->cmp_func(cur->node, node) != 0) {
		cur = cur->next;
	}
	if (cur != NULL) {
		return cur->node;
	}
	return NULL;
}

/**
 * Gets head element and remove it from the list.
 *
 * @param list Pointer to the list to use.
 *
 * @return Pointer to the element or NULL if the list is empty.
 */
void *list_pophead(t_list *list)
{
	void *node;
	t_listnode *head;

	XASSERT(list != NULL, NULL);

	if (list->head == NULL) {
		return NULL;
	}
	head = list->head;
	list->head = head->next;
	if (list->head == NULL) {
		list->tail = NULL;
	} else {
		list->head->prev = NULL;
	}
	node = head->node;
	free(head);
	list->size--;
	return node;
}

/**
 * Gets a list node out of a list.
 *
 * @param list Pointer to the list to use.
 * @param node Pointer to the list node to pop.
 *
 * @return 0 on success, or -1 if an error occurs.
 */
int list_popnode(t_list *list, t_listnode *node)
{
	XASSERT(list != NULL, -1);
	XASSERT(node != NULL, -1);

	if (node->prev == NULL) {
		list->head = node->next;
	} else {
		node->prev->next = node->next;
	}
	if (node->next == NULL) {
		list->tail = node->prev;
	} else {
		node->next->prev = node->prev;
	}
	node->prev = NULL;
	node->next = NULL;
	list->size--;
	return 0;
}

/**
 * Gets head element of a list. Keeps the element in the list.
 *
 * @param list Pointer to the list to use.
 *
 * @return Pointer to the element found or NULL if the list is empty.
 */
void *list_gethead(t_list *list)
{
	XASSERT(list != NULL, NULL);

	if (list->head == NULL) {
		return NULL;
	}
	return list->head->node;
}

/**
 * Gets next element of a list. This element will be stored in
 * a list iterator to easily access to the next element in the
 * next call.
 *
 * @param list Pointer to the list to use.
 * @param iterator Pointer to a list iterator where to store the current element.
 *
 * @return A pointer to the current element or NULL if the end is reached.
 */
void *list_getnext(t_list *list, t_listiterator *iterator)
{
	XASSERT(list != NULL, NULL);
	XASSERT(iterator != NULL, NULL);

	if (unlikely(*iterator == NULL)) {
		*iterator = list->head;
	} else {
		*iterator = (*iterator)->next;
	}
	if (unlikely(*iterator == NULL)) {
		return NULL;
	}
	return (*iterator)->node;
}
