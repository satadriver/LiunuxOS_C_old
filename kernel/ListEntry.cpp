
#include "ListEntry.h"


void initListEntry(LIST_ENTRY * list) {
	list->next = list;
	list->prev = list;
}

//add after head
void addlistHead(LIST_ENTRY * head, LIST_ENTRY * list) {
	if (head == list)
	{
		head->next = list;
		head->prev = list;
		return;	
	}

	LIST_ENTRY * headnext = head->next;

	list->prev = head;
	list->next = headnext;

	head->next = list;
	headnext->prev = list;
}

//add before head
void addlistTail(LIST_ENTRY * head, LIST_ENTRY * list) {
	if (head == list)
	{
		head->next = list;
		head->prev = list;
		return;
	}
	LIST_ENTRY * headprev = head->prev;

	list->prev = headprev;
	list->next = head;

	headprev->next = list;
	head->prev = list;
}

int existlist(LIST_ENTRY * head, LPLIST_ENTRY list) {
	if (head == list)
	{
		return TRUE;
	}

	LPLIST_ENTRY e = head->next;
	while (e && (e != head))
	{
		if (e == list)
		{
			return TRUE;
		}
		else {
			e = e->next;
		}
	}

	return FALSE;
}


void removelist(LPLIST_ENTRY list) {

	if (list->next == 0 && list->prev == 0)
	{
		return;
	}else if (list->next == 0)
	{
		list->prev->next = 0;
	}
	else if (list->prev == 0)
	{
		list->next->prev = 0;
	}
	else {
		list->prev->next = list->next;
		list->next->prev = list->prev;
	}
}