#include <stdlib.h>
#include "adlist.h"

list *listCreate(void){
	list *list = malloc(sizeof(struct list));
	if ( list == NULL ) return NULL;
	list->head = NULL;
	list->tail = NULL;
	list->len = 0;
	list->dup = NULL;
	list->free = NULL;
	list->match = NULL;
	return list;
}
void listRelease(list *list){
	if ( list == NULL ) return;
	listNode *current, *next;
	current = list->head;
	unsigned long len;
	len = list->len;
	while(len--){
		next = current->next;
		if (list->free ) list->free(current->value);
		free(current);
		current = next;
	}
	free(list);
	list = NULL;
}
list *listAddNodeHead(list *list, void *value){
	listNode *node = malloc(sizeof(listNode));
	if ( node == NULL ) return NULL;
	node->value = value;
	if ( list->len == 0  ){
		list->head = list->tail = node;
		node->next=node->prev=NULL;
	}else{
		node->prev = NULL;
		node->next = list->head;
		list->head->prev = node;
		list->head = node;
	}
	list->len ++;
	return list;
}
list *listAddNodeTail(list *list, void *value){
}

#include <stdio.h>
#include "testhelp.h"

int main(void){
	list *list = listCreate();
	test_cond("list_create",list != NULL );
	listRelease(list);

	listNode *node = listFirst(list);
	return 0;
}
