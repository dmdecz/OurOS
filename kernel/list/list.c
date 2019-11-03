#include "list.h"

uint get_size(list *list)
{
	if (!list)
		return 0;
	return list->size;
}

list_node *get_head(list *list)
{
	if (!list)
		return nullptr;
	return list->head;
}

list_node *get_prev(list_node *node)
{
	if (!node)
		return nullptr;
	return node->prev;
}

list_node *get_next(list_node *node)
{
	if (!node)
		return nullptr;
	return node->next;
}

list_node *find_list_node(list *list, uint index)
{
	list_node *ret = nullptr;
	if (list && index < list->size) {
		int go_back = 0;
		list_node *p = list->head;
		if (index > list->size / 2)
			go_back = 1;
		if (go_back) {
			index = list->size - index;
			while (index--) {
				p = p->prev;
			}
		} else {
			while (index--) {
				p = p->next;
			}
		}
		ret = p;
	}
	return ret;
}

int insert_list_node(list *list, list_node *node, uint index)
{
	if (!list || !node)
		return 0;
	if (index > list->size)
		return 0;
	if (list->size == 0)
		return append_list_node(list, node);
	if (index == 0) {
		int ret = append_list_node(list, node);
		if (ret)
			list->head = list->head->prev;
		return ret;
	}
	if (index == list->size)
		return append_list_node(list, node);
	list_node *p = find_list_node(list, index);
	list_node *head = list->head;
	list->head = p;
	int ret = insert_list_node(list, node, 0);
	list->head = head;
	return ret;
}

int append_list_node(list *list, list_node *node)
{
	int ret = 0;
	if (!list || !node) {
		ret = 0;
	} else if (list->size == 0) {
		node->next = node;
		node->prev = node;
		list->head = node;
		ret = 1;
	} else {
		node->next = list->head;
		node->prev = list->head->prev;
		node->prev->next = node;
		list->head->prev = node;
		ret = 1;
	}
	list->size += ret;
	return ret;
}

list_node *delete_list_node(list *list, uint index)
{
	if (!list || list->size == 0 || index >= list->size)
		return nullptr;
	if (index == 0 || list->size == 1) {
		list->head = list->head->next;
		return pop_list_node(list);
	}

	list_node *head = list->head;
	list_node *p = find_list_node(list, index);
	list->head = p->next;
	list_node *ret = pop_list_node(list);
	list->head = head;
	return ret;
}

list_node *pop_list_node(list *list)
{
	if (!list || list->size == 0)
		return nullptr;
	list_node *ret = nullptr;
	if (list->size == 1) {
		ret = list->head;
		list->head = nullptr;
	} else {
		ret = list->head->prev;
		ret->prev->next = ret->next;
		ret->next->prev = ret->prev;
	}

	list->size -= ret != nullptr;
	return ret;
}

void traversal_list(list *list, void (*f)(list_node *))
{
	if (!list)
		return;
	list_node *p = list->head;
	uint size = list->size;
	while (size--) {
		f(p);
		p = p->next;
	}
}