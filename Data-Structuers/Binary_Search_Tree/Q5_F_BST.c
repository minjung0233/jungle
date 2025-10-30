//////////////////////////////////////////////////////////////////////////////////

/* CE1007/CZ1007 Data Structures
Lab Test: Section F - Binary Search Trees Questions
Purpose: Implementing the required functions for Question 5
		 Implementing 'remove node' operation for BST*/
//////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////////////

typedef struct _bstnode{
	int item;
	struct _bstnode *left;
	struct _bstnode *right;
} BSTNode;   // You should not change the definition of BSTNode

typedef struct _stackNode{
	BSTNode *data;
	struct _stackNode *next;
}StackNode; // You should not change the definition of StackNode

typedef struct _stack
{
	StackNode *top;
}Stack; // You should not change the definition of Stack

///////////////////////// function prototypes ////////////////////////////////////

// You should not change the prototypes of these functions
void postOrderIterativeS2(BSTNode *root);

void insertBSTNode(BSTNode **node, int value);

void push(Stack *stack, BSTNode *node);
BSTNode *pop(Stack *s);
BSTNode *peek(Stack *s);
int isEmpty(Stack *s);
void removeAll(BSTNode **node);
BSTNode* removeNodeFromTree(BSTNode *root, int value);

///////////////////////////// main() /////////////////////////////////////////////

int main()
{
	int c, i;
	c = 1;

	//Initialize the Binary Search Tree as an empty Binary Search Tree
	BSTNode * root;
	root = NULL;

	printf("1: Insert an integer into the binary search tree;\n");
	printf("2: Print the post-order traversal of the binary search tree;\n");
	printf("0: Quit;\n");


	while (c != 0)
	{
		printf("Please input your choice(1/2/0): ");
		scanf("%d", &c);

		switch (c)
		{
		case 1:
			printf("Input an integer that you want to insert into the Binary Search Tree: ");
			scanf("%d", &i);
			insertBSTNode(&root, i);
			break;
		case 2:
			printf("The resulting post-order traversal of the binary search tree is: ");
			postOrderIterativeS2(root); // You need to code this function
			printf("\n");
			break;
		case 0:
			removeAll(&root);
			break;
		default:
			printf("Choice unknown;\n");
			break;
		}

	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////
// 후위 순회 - 왼,오,루
// 스택 두 개로
void postOrderIterativeS2(BSTNode *root)
{
	// 후위 순회 - 왼,오,루

	// 왼쪽 자식, 오른쪽 자식을 가져와서 임시저장 하는 스택? >> 순서를 반대로 
	// 먼저 push 된것이 먼저 출력
	Stack s1 = {s1.top = NULL};

	// 최종 출력할 스택
	// 먼저 push된것이 나중에 출력
	Stack s2 = {s2.top = NULL};

	// 루트 넣기
	push(&s1,root);

	while(!(isEmpty(&s1))){
		// 스택 1에서 꺼내 스택2로
		root = pop(&s1);
		push(&s2, root);
		// 오른쪽 자식 먼저 출력되기 위해 오른쪽을 s1에 먼저 push한 후 s2에 넣기
		// 
		if(root->left){
			// 스택 1에서 꺼낸 노드의 왼쪽 자식을 스택 1에 넣기
			push(&s1, root->left);
		}
		if (root->right){
			// 왼쪽 자식보다 오른쪽 자식을 먼저 스택1에 넣기
			push(&s1, root->right);
		}

	}
	// 순서 반대로해서 최종 출력
	while(!(isEmpty(&s2))){
		BSTNode *node = pop(&s2);
		if(node != NULL){
			int value = node->item;
			printf("%d ", value);
		}
	}
	root = removeNodeFromTree(root, root->item);
	
}

/* Given a binary search tree and a key, this function
   deletes the key and returns the new root. Make recursive function. */
BSTNode* removeNodeFromTree(BSTNode *root, int value)
{
	
	
	// BST의 규칙으로 value값 찾아서 삭제
	// 루트 보다 작으면 왼쪽 트리
	if(root->item > value){
		root->left = removeNodeFromTree(root->left, value);
	}
	// 루트보다 크면 오른쪽 트리
	else if(root->item < value){
		root->right = removeNodeFromTree(root->right, value);
	}
	// 루트값과 value값이 같다면 삭제
	else{
		// 1. 자식이 없으면
		if(root->left == NULL && root->right == NULL){
			free(root);
			return NULL;
		}
		// 2. 자식이 하나일 경우
		else if(root->left == NULL){
			BSTNode *temp = root->right;
			free(root);
			return temp; // 오른쪽 자식이 새 루트
		}
		else if (root->right == NULL){
			BSTNode *temp = root->left;
			free(root);
			return temp; // 왼쪽 자식이 새 루트
		}
		// 3. 자식이 둘 다 있는 경우
		else{
			BSTNode *node = root->right;
			// BST의 특징인 왼<루<오를 유지하기 위해
			// 오른쪽 가장 작은 값 (or 왼쪽 가장 큰 값) 이용해서 root로 만들기
			while (node->left != NULL){
				node = node->left;
			}
			// 후계자의 값 복사
			root->item = node->item;

			// 오른쪽 서브트리에서 후계자 제거(root로 된 노드, 원래 위치 제거)
			root->right = removeNodeFromTree(root->right, node->item);
		}
	}
	return root;
}
///////////////////////////////////////////////////////////////////////////////

void insertBSTNode(BSTNode **node, int value){
	if (*node == NULL)
	{
		*node = malloc(sizeof(BSTNode));

		if (*node != NULL) {
			(*node)->item = value;
			(*node)->left = NULL;
			(*node)->right = NULL;
		}
	}
	else
	{
		if (value < (*node)->item)
		{
			insertBSTNode(&((*node)->left), value);
		}
		else if (value >(*node)->item)
		{
			insertBSTNode(&((*node)->right), value);
		}
		else
			return;
	}
}

//////////////////////////////////////////////////////////////////////////////////

void push(Stack *stack, BSTNode * node)
{
	StackNode *temp;

	temp = malloc(sizeof(StackNode));

	if (temp == NULL)
		return;
	temp->data = node;

	if (stack->top == NULL)
	{
		stack->top = temp;
		temp->next = NULL;
	}
	else
	{
		temp->next = stack->top;
		stack->top = temp;
	}
}


BSTNode * pop(Stack * s)
{
	StackNode *temp, *t;
	BSTNode * ptr;
	ptr = NULL;

	t = s->top;
	if (t != NULL)
	{
		temp = t->next;
		ptr = t->data;

		s->top = temp;
		free(t);
		t = NULL;
	}

	return ptr;
}

BSTNode * peek(Stack * s)
{
	StackNode *temp;
	temp = s->top;
	if (temp != NULL)
		return temp->data;
	else
		return NULL;
}

int isEmpty(Stack *s)
{
	if (s->top == NULL)
		return 1;
	else
		return 0;
}


void removeAll(BSTNode **node)
{
	if (*node != NULL)
	{
		removeAll(&((*node)->left));
		removeAll(&((*node)->right));
		free(*node);
		*node = NULL;
	}
}
