//////////////////////////////////////////////////////////////////////////////////

/* CE1007/CZ1007 Data Structures
Lab Test: Section A - Linked List Questions
Purpose: Implementing the required functions for Question 6 */

//////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////////////

typedef struct _listnode
{
	int item;
	struct _listnode *next;
} ListNode;			// You should not change the definition of ListNode

typedef struct _linkedlist
{
	int size;
	ListNode *head;
} LinkedList;			// You should not change the definition of LinkedList


//////////////////////// function prototypes /////////////////////////////////////

// You should not change the prototype of this function
int moveMaxToFront(ListNode **ptrHead);

void printList(LinkedList *ll);
void removeAllItems(LinkedList *ll);
ListNode * findNode(LinkedList *ll, int index);
int insertNode(LinkedList *ll, int index, int value);
int removeNode(LinkedList *ll, int index);


//////////////////////////// main() //////////////////////////////////////////////

int main()
{
	int c, i, j;
	c = 1;

	LinkedList ll;
	//Initialize the linked list 1 as an empty linked list
	ll.head = NULL;
	ll.size = 0;


	printf("1: Insert an integer to the linked list:\n");
	printf("2: Move the largest stored value to the front of the list:\n");
	printf("0: Quit:\n");

	while (c != 0)
	{
		printf("Please input your choice(1/2/0): ");
		scanf("%d", &c);

		switch (c)
		{
		case 1:
			printf("Input an integer that you want to add to the linked list: ");
			scanf("%d", &i);
			j=insertNode(&ll, ll.size, i);
			printf("The resulting linked list is: ");
			printList(&ll);
			break;
		case 2:
			moveMaxToFront(&(ll.head));  // You need to code this function
			printf("The resulting linked list after moving largest stored value to the front of the list is: ");
			printList(&ll);
			removeAllItems(&ll);
			break;
		case 0:
			removeAllItems(&ll);
			break;
		default:
			printf("Choice unknown;\n");
			break;
		}
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////

int moveMaxToFront(ListNode **ptrHead)
{
	/*
	1. temp를 이용해 리스트를 순회하며 기준(max)보다 큰 값을 찾으면 
	 >> max에 temp값으로 갱신 + 앞(전) 값 갱신
	2. 큰 값을 앞으로
	 >> 기존의 큰 값 지우기 -> 이때 front 노드 사용해서 front->next를 큰값이 원래 가르켰던 노드(max->next) 가르키기
	 >> 가장 큰 값 맨 앞으로 옯기기 -> 현재 max는 가장 큰 값을 가진 노드, max->next를 기존의 앞에 있던 ptrHead를 가르키기
	 								+ head를 max노드로 바꾸기
	*/
    // 가장 입력되는 가장 큰수를 맨 앞으로
	// LinkedList의 head를 가르키는 ptrHead
	// front - 가장 큰 노드의 앞, max - 가장 큰 노드를 가르키는 노드
	// temp - 순회하는 노드, prev - temp의 이전 노드(즉, front 값)
	ListNode *front = NULL;
	ListNode *max;
	ListNode *temp;
	ListNode *prev;

	temp = *ptrHead;
	max = *ptrHead;
	while(temp != NULL){
		// 큰 값 찾기
		// temp->item : 순회하는 값
		// max->item : 현재까지 나온 노드 중 가장 큰 값
		if(temp->item > max->item){
			max = temp;
			front = prev;
		}
		// 다음 노드 갱신
		prev = temp;
		temp = temp->next;
	}
	
	// 예외 처리
	// - 만약 맨 앞에 수가 가장 크다면 
	// - 노드가 하나라면 
	if(front == NULL || max == *ptrHead) { return -1; }

	// 2. 큰 값을 앞으로
	if(front->next->item == max->item){
		// 기존의 max 값 지우기
		front->next = max->next;
		// 가장 큰 값 맨앞으로 옮기기
		max->next = *ptrHead;
		*ptrHead = max;
	}
}

//////////////////////////////////////////////////////////////////////////////////

void printList(LinkedList *ll){

	ListNode *cur;
	if (ll == NULL)
		return;
	cur = ll->head;

	if (cur == NULL)
		printf("Empty");
	while (cur != NULL)
	{
		printf("%d ", cur->item);
		cur = cur->next;
	}
	printf("\n");
}

ListNode * findNode(LinkedList *ll, int index){

	ListNode *temp;

	if (ll == NULL || index < 0 || index >= ll->size)
		return NULL;

	temp = ll->head;

	if (temp == NULL || index < 0)
		return NULL;

	while (index > 0){
		temp = temp->next;
		if (temp == NULL)
			return NULL;
		index--;
	}

	return temp;
}

int insertNode(LinkedList *ll, int index, int value){

	ListNode *pre, *cur;

	if (ll == NULL || index < 0 || index > ll->size + 1)
		return -1;

	// If empty list or inserting first node, need to update head pointer
	if (ll->head == NULL || index == 0){
		cur = ll->head;
		ll->head = malloc(sizeof(ListNode));
		ll->head->item = value;
		ll->head->next = cur;
		ll->size++;
		return 0;
	}


	// Find the nodes before and at the target position
	// Create a new node and reconnect the links
	if ((pre = findNode(ll, index - 1)) != NULL){
		cur = pre->next;
		pre->next = malloc(sizeof(ListNode));
		pre->next->item = value;
		pre->next->next = cur;
		ll->size++;
		return 0;
	}

	return -1;
}


int removeNode(LinkedList *ll, int index){

	ListNode *pre, *cur;

	// Highest index we can remove is size-1
	if (ll == NULL || index < 0 || index >= ll->size)
		return -1;

	// If removing first node, need to update head pointer
	if (index == 0){
		cur = ll->head->next;
		free(ll->head);
		ll->head = cur;
		ll->size--;

		return 0;
	}

	// Find the nodes before and after the target position
	// Free the target node and reconnect the links
	if ((pre = findNode(ll, index - 1)) != NULL){

		if (pre->next == NULL)
			return -1;

		cur = pre->next;
		pre->next = cur->next;
		free(cur);
		ll->size--;
		return 0;
	}

	return -1;
}

void removeAllItems(LinkedList *ll)
{
	ListNode *cur = ll->head;
	ListNode *tmp;

	while (cur != NULL){
		tmp = cur->next;
		free(cur);
		cur = tmp;
	}
	ll->head = NULL;
	ll->size = 0;
}
