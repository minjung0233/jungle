#include "rbtree.h"

#include <stdlib.h>
/////////////////////////////
//(확인용)
#include <stdio.h>

node_t *p_tree_min(rbtree *t, node_t *node);
// 회전
void left_rotate(rbtree *t, node_t *x);
void right_rotate(rbtree *t, node_t *x);

void inorder(const rbtree *t, key_t *arr, node_t *node, const size_t n, size_t *index); // 중위 순회로 배열 만들기
void delete_node(rbtree *t, node_t *node);


/////////////////////////////
rbtree *new_rbtree(void) {
    rbtree *p = malloc(sizeof(rbtree));
    p->nil = malloc(sizeof(node_t));
    p->nil->color = RBTREE_BLACK;
    p->root= p->nil;
    return p;
}

//////////////////////////////////////
//확인용

void delete_node(rbtree *t, node_t *node) {
  // 후위 순회로 재귀적으로 지우기
    if (node == t->nil) return;
    delete_node(t,node->left);
    delete_node(t,node->right);
    free(node);

}

void delete_rbtree(rbtree *t) {
  if(t = NULL) return;
  if (t->root != t->nil)
    delete_node(t, t->root);
  free(t->nil);
  free(t);
}

void rbtree_insert_fixup(rbtree *t,node_t *z){
  /*
    조건(rbtree의 속성이 모두 만족한다면) 상황 종료
    루트는 레드이고 레드가 연속적이면 계속 돌기(될때까지 바꾸기) -> 속성 2,4를 위반하였을 때
    insert_n의 경우는 3가지
    1. 새로 들어온 값
    2. 속성 4를 위반해서 case 2후 case 3을 돌아야할 경우
    3. case 1 후 조상 노드(조상 노드 재검사할 경우)
    이때 1과 2는 list_n의 값이 똑같이 추가된 값임
  */
  
  while(z->parent->color == RBTREE_RED){
    node_t *y;
    if(z->parent == z->parent->parent->left){
      y = z->parent->parent->right;
      // case 1
      if(y->color == RBTREE_RED){
        // 색바꾸기
        z->parent->color = RBTREE_BLACK;
        y->color = RBTREE_BLACK;
        z->parent->parent->color = RBTREE_RED;

        // 할아버지 확인
        z = z->parent->parent;
        continue;
      }
      else{
        // case 2
        if(z == z->parent->right){
          z = z->parent;
          left_rotate(t,z);
        }
        // case 3
        z->parent->color = RBTREE_BLACK;
        z->parent->parent->color = RBTREE_RED;
        right_rotate(t,z->parent->parent);
      }
    }
    else{
      y = z->parent->parent->left;

      if(y->color == RBTREE_RED){
        z->parent->color = RBTREE_BLACK;
        y->color = RBTREE_BLACK;
        z->parent->parent->color = RBTREE_RED;
        z = z->parent->parent;
      }
      else{
        if(z == z->parent->left){
          z = z->parent;
          right_rotate(t,z);
        }
        z->parent->color = RBTREE_BLACK;
        z->parent->parent->color = RBTREE_RED;
        left_rotate(t,z->parent->parent);
      }
    }
  }
  t->root->color = RBTREE_BLACK;
}

node_t *rbtree_insert(rbtree *t, const key_t key) {
  node_t *z = (node_t *)malloc(sizeof(node_t));
  z->key = key;

  node_t *x = t->root;
  node_t *y = t->nil;

  // nil까지 
  while(x!=t->nil){
    y=x;
    if(z->key < x->key){
      x = x->left;
    }
    else{
        x = x->right;
    }
  }
  z->parent = y;
  if(y==t->nil){
    t->root = z;
  }
  else if(z->key < y->key){
    y->left = z;
  }
  else{
    y->right = z;
  }
  z->left = t->nil;
  z->right = t->nil;
  z->color = RBTREE_RED;

  rbtree_insert_fixup(t, z);
  return t->root;
}


// 트리에 key가 있으면 해당 node pointer 반환, 없으면 NULL
node_t *rbtree_find(const rbtree *t, const key_t key) {
  // 순회하는 포인터 temp
  node_t *temp = t->root;
  
  // 1. t를 돌면서
  while(temp != t->nil){
    // 1-1. key 값과 같음 return
    if(key > temp->key) { 
      temp = temp->right;
      
    }
    // 1-2. key값이 temp 보다 작으면 temp를 왼쪽으로
    else if(key < temp->key){
      temp = temp->left;
    }
    // 1-3. key값이 temp 보다 크면 temp를 오른쪽으로
    else{
      return temp;
    }
  }
  return NULL;
  
}

node_t *rbtree_min(const rbtree *t) {
  node_t *cur = t->root;
  node_t *min_val;
  while(cur != t->nil){
    min_val = cur;
    cur = cur->left;
  }
  return min_val;
}

node_t *rbtree_max(const rbtree *t) {
  node_t *cur = t->root;
  node_t *max_val;
  while(cur != t->nil){
    max_val = cur;
    cur = cur->right;
  }
  return max_val;
}
//////////////////////
// u -> v
void rb_transplant(rbtree *t, node_t *u, node_t *v){
  if(u->parent == t->nil){
    t->root = v;
  }
  else if(u == u->parent->left){
    u->parent->left = v;
  }
  else{
    u->parent->right = v;
  }
  v->parent = u->parent;
}
void rb_delete_fixup(rbtree *t, node_t *x){
  node_t *w;
  while(x != t->root && x->color == RBTREE_BLACK){
    if(x == x->parent->left){
      w = x->parent->right;
      // case 1
      if(w->color == RBTREE_RED){
        w->color = RBTREE_BLACK;
        x->parent->color = RBTREE_RED;
        left_rotate(t,x->parent);
        w = x->parent->right; 
      }
      // case 2
      if(w->left->color == RBTREE_BLACK && w->right->color == RBTREE_BLACK){
        w->color = RBTREE_RED;
        x= x->parent;
      }
      // case 3
      else{
        if(w->right->color == RBTREE_BLACK){
          w->left->color = RBTREE_BLACK;
          w->color = RBTREE_RED;
          right_rotate(t,w);
          w = x->parent->right;
        }
        // case 4
        w->color = x->parent->color;
        x->parent->color = RBTREE_BLACK;
        w->right->color = RBTREE_BLACK;
        left_rotate(t,x->parent);
        x = t->root;
      }
    }
    else{
      w = x->parent->left;
      if(w->color == RBTREE_RED){
        w->color = RBTREE_BLACK;
        x->parent->color = RBTREE_RED;
        right_rotate(t,x->parent);
        w = x->parent->left;
      }
      if(w->right->color == RBTREE_BLACK && w->left->color == RBTREE_BLACK){
        w->color = RBTREE_RED;
        x = x->parent;
      }
      else{
        if(w->left->color == RBTREE_BLACK){
          w->right->color = RBTREE_BLACK;
          w->color = RBTREE_RED;
          left_rotate(t,w);
          w = x->parent->left;
        }
        w->color = x->parent->color;
        x->parent->color = RBTREE_BLACK;
        w->left->color = RBTREE_BLACK;
        right_rotate(t, x->parent);
        x = t->root;
      }
    }
  }
  x->color = RBTREE_BLACK;
}

void left_rotate(rbtree *t, node_t *x){
  node_t *y = x->right;

  x->right = y->left;
  // 부모(x)의 왼쪽 서브트리를 조상(y)의 오른쪽에 붙임  
  if(y->left != t->nil){
    y->left->parent = x;
  }
  // 부모를 조상 위치로 올림
  y->parent = x->parent;
  if(x->parent == t->nil){
    t->root = y;
  }
  else if(x == x->parent->left){
    x->parent->left = y;
  }
  else{
    x->parent->right = y;
  }
  // 부모/자식 연결
  y->left = x;
  x->parent = y;
}

void right_rotate(rbtree *t, node_t *y){
  node_t *x = y->left;

  y->left = x->right;
  // 부모(x)의 오른쪽 서브트리를 조상(y)의 왼쪽에 붙임
  if(x->right != t->nil){
    x->right->parent = y;
  }

  // 부모를 조상 위치로 올림
  x->parent = y->parent;
  if(y->parent == t->nil){
    t->root = x;
  }
  else if(y == y->parent->left){
    y->parent->left = x;
  }
  else{
    y->parent->right = x;
  }

  // 부모/자식 연결
  x->right = y;
  y->parent = x;
}

int rbtree_erase(rbtree *t, node_t *p) {
  node_t *y = p;
  color_t y_original_color = y->color;
  node_t *x;
  if(p->left == t->nil){
    x = p->right;
    rb_transplant(t,p,p->right);  // 오른쪽 자식 바꾸기
  }
  else if(p->right == t->nil){
    x = p->left;
    rb_transplant(t,p,p->left);
  }
  else{
    y = p_tree_min(t, p->right);   // y는 p의 후손
    y_original_color = y->color;
    x = y->right;
    // 더 아래쪽으로
    if(y != p->right){
      rb_transplant(t,y,y->right);
      y->right = p->right;
      y->right->parent = y;
    }
    else{
      x->parent = y;
    }
    rb_transplant(t,p,y);
    y->left = p->left;
    y->left->parent = y;
    y->color = p->color;
  }
  if(y_original_color == RBTREE_BLACK){
    rb_delete_fixup(t,x);
  }
  free(p);
 return 0;
}

// 주어진 arr에 오름차순으로 넣기(중위순회)
// 최대 n개 까지
int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n) {
  size_t index = 0;
  inorder(t,arr,t->root,n, &index);
  return 0;
}
void inorder(const rbtree *t, key_t *arr, node_t *node, const size_t n, size_t *index){
  if (node == t->nil || *index >= n){
    return;
  }
  inorder(t, arr, node->left,n,index);
  arr[(*index)++]=node->key;
  inorder(t, arr, node->right, n,index);
}

node_t *p_tree_min(rbtree *t, node_t *node){
  while(node->left != t->nil){
    node = node->left;
  }
  return node;
}