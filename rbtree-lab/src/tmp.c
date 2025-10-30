#include <stdio.h>
#include "rbtree.h" // node_t, rbtree, color 정의되어 있다고 가정

void print_subtree(const node_t *node, const node_t *nil, const char *prefix, int is_left) {
    if (node == nil)
        return;

    printf("%s", prefix);

    // ├── or └── 출력
    printf("%s", is_left ? "├── " : "└── ");

    // key와 색 출력
    printf("key: %d(%c)\n", node->key, node->color == RBTREE_RED ? 'R' : 'B');

    // 다음 prefix 만들기
    char new_prefix[256];
    snprintf(new_prefix, sizeof(new_prefix), "%s%s", prefix, is_left ? "│   " : "    ");

    // 왼쪽, 오른쪽 순서로 재귀 호출
    print_subtree(node->left, nil, new_prefix, 1);
    print_subtree(node->right, nil, new_prefix, 0);
}

void print_tree(const rbtree *t) {
    if (t == NULL || t->root == t->nil) {
        printf("(empty tree)\n");
        return;
    }
    printf("key: %d(%c)\n", t->root->key, t->root->color == RBTREE_RED ? 'R' : 'B');
    print_subtree(t->root->left, t->nil, "", 1);
    print_subtree(t->root->right, t->nil, "", 0);
}
