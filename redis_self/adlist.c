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
void listDelNode(list *list, listNode *node){
	if ( list == NULL ) return;
	if ( node->prev != NULL ){
		node->prev->next = node->next;
	}else{
		list->head = node->next;
	}
	if ( node->next != NULL ){
		node->next->prev= node->prev;
	}else{
		list->tail = node->prev;
	}
	--list->len;
	if ( list->free ) list->free(node->value);
	free(node);
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
	listNode *node = malloc(sizeof(listNode));
	if ( node == NULL ) return NULL;
	node->value = value;
	if ( list->len == 0  ){
		list->head = list->tail = node;
		node->next=node->prev=NULL;
	}else{
		node->prev = list->tail;
		node->next = NULL;
		list->tail->next= node;
		list->tail= node;
	}
	list->len ++;
	return list;
}
list *listInsertNode(list *list, listNode *old_node, void *value, int after){
	if ( list == NULL ) return NULL;
	if ( value == NULL ) return list;
	listNode *node = malloc(sizeof(listNode));
	if ( node == NULL ) return NULL;
	node->value = value;
	if ( after ){
		node->prev = old_node;
		node->next = old_node->next;
		old_node->next = node;
		if ( old_node == listLast(list) ){
			list->tail = node;
		}
	}else{
		node->next = old_node;
		node->prev = old_node->prev;
		old_node->prev = node;
		if ( old_node == listFirst(list)){
			list->head = node;
		}
	}
	list->len ++;
	return list;
}
listIter *listGenIterator(list *list, int direction){
	listIter *retVal = NULL;
	if ( (retVal = malloc(sizeof(listIter))) == NULL ) return NULL;
	if ( direction == AL_START_HEAD )
		retVal->next = list->head;
	else
		retVal->next = list->tail;
	retVal->direction = direction;
	return retVal;
}
listNode *listNext(listIter *iter){
	if ( iter == NULL ) return NULL;
	listNode *retVal = iter->next;
	if ( iter->next != NULL ){
		if ( iter->direction == AL_START_HEAD )
			iter->next = iter->next->next;
		else
			iter->next = iter->next->prev;
	}
	return retVal;
}
void listReleaseIterator(listIter *iter){
	free(iter);
}
list *listDup(list *orig){
	list* copy= listCreate();
	copy->dup = orig->dup;
	copy->free = orig->free;
	copy->match = orig->match;
	listIter *iter = listGenIterator(orig,AL_START_HEAD);
	listNode *node = NULL;
	while((node = listNext(iter))!= NULL ){
		void *value;
		if ( orig->dup != NULL ){
			value = orig->dup(node->value);
			if ( value == NULL ){
				listRelease(copy);
				listReleaseIterator(iter);
			}
		}else{
			value = node->value;
		}

		if ( listAddNodeTail(copy, value ) == NULL ){
			listRelease(copy);
			listReleaseIterator(iter);
			return NULL;
		}
	}
	listReleaseIterator(iter);

	return copy;
}
listNode *listSearchKey(list *list, void *key){
}

#include <string.h>
#include "testhelp.h"
#include <stdio.h>


void testFree(void *ptr){
	free(ptr);
}
void *testDup(void *ptr){
	void *copy = malloc(sizeof(*ptr));
	memcpy(copy,ptr,sizeof(*ptr));
	return copy;
}

int main(void){
	list* testList = listCreate();
	test_cond("list_create",testList != NULL );
	testList->free = testFree;

// head is malloc  but tail is localParam see printf tail value.... is ........hard to understand;
	int *intHeadVal = malloc(sizeof(int));
	*intHeadVal = 21;
	testList = listAddNodeHead(testList,(void *)intHeadVal);
	int *intLastVal = malloc(sizeof(int));
	*intLastVal  = 31;
	testList = listAddNodeTail(testList,(void *)intLastVal);
	listNode *node = listFirst(testList);
	int* iVal = (int * ) (node->value);
	int* lastiVal = (int * ) (listLast(testList)->value);
	test_cond("testList add head", *iVal == 21);
	iVal = listLast(testList)->value;
	test_cond("testList add tail", *iVal == 31);
	listIter *iter = listGenIterator(testList,AL_START_HEAD);
	test_cond("iter", iter->next == testList->head);
	listNext(iter);
	test_cond("iter", iter->next == testList->tail);
	testList->dup = testDup;
	list* copyList;
	copyList = listDup(testList);
	iVal = (int *)(listFirst(copyList)->value);
	test_cond("copylist head", *iVal == 21);
	iVal = (int *)(listLast(copyList)->value);
	test_cond("copylist tail", *iVal == 31);
	listRelease(copyList);
	listRelease(testList);
	return 0;
}
