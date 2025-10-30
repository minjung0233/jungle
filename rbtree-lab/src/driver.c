#include "rbtree.h"
#include <stdio.h>
#include <stdlib.h>
#include "tmp.c"

int main(int argc, char *argv[]) {
    // rbtree 생성
    rbtree *t = new_rbtree();
    
    printf("값을 입력하세요 (-1 입력 시 종료):\n");

    // rbtree에 노드 삽입
    int value = 0;
    while(1){
        scanf("%d", &value);
        if(value == -1) break;
        rbtree_insert(t, value);
        
    }
    
    // 임시 프린트(확인용)
    // print_inorder(t->root, t->nil);
    printf("\n");
    print_tree(t);
    int n = 14;
    printf("\n여기서부턴 배열 출력\n");
    key_t arr1[] = {10, 5, 8, 34, 67, 23, 156, 24, 2, 12, 24, 36, 990, 25};
    key_t *res = calloc(n, sizeof(key_t));
    rbtree_to_array(t, res, n);
    for (int i = 0; i < n; i++) {
        printf("%d ", res[i]);
        // if(arr1[i] == res[i]){
        //     printf("%d ", res[i]);
        // }
    }
    free(res);
    printf("\n");

    // int delete_value = 0;
    
    // while(1){
    //     scanf("%d", &delete_value);
    //     if(delete_value == -1) break;
    //     node_t *find_node = rbtree_find(t, delete_value);
    //     if (find_node == NULL) {
    //         printf("key %d not found in tree\n", delete_value);
    //         // continue;  //
    //         break;
    //     }
    //     printf("find_node : %d", find_node->key);
    //     int result = rbtree_erase(t, find_node);
    //     printf("result %d\n", result);
    //     // print_inorder(t->root, t->nil);
    //     print_tree(t);
    // }
    // printf("delete:\n");
    // // print_inorder(t->root, t->nil);
    // print_tree(t);
    // printf("\n");
    // 임시 삭제 함수
    delete_rbtree(t);
    // 메모리 누수 확인 - valgrind
    // valgrind ./my_program.out
}
