#include "node.h"

#include <stdio.h>
#include <stdlib.h>

Node *node_create(uint8_t symbol, double weight) {
    Node *new_node = (Node *) malloc(sizeof(Node));
    if (new_node == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for new node.\n");
        exit(1);
    }

    new_node->symbol = symbol;
    new_node->weight = weight;
    new_node->code = 0;
    new_node->code_length = 0;
    new_node->left = NULL;
    new_node->right = NULL;

    return new_node;
}

void node_free(Node **node) {
    if (*node == NULL) {
        return;
    }

    node_free(&(*node)->left);
    node_free(&(*node)->right);

    free(*node);
    *node = NULL;
}

void node_print_tree(Node *tree, char ch, int indentation) {
    if (tree == NULL) {
        return;
    }

    node_print_tree(tree->right, '/', indentation + 3);
    printf("%*cweight = %.0f", indentation + 1, ch, tree->weight);

    if (tree->left == NULL && tree->right == NULL) {
        if (' ' <= tree->symbol && tree->symbol <= '~') {
            printf(", symbol  = '%c'", tree->symbol);
        } else {
            printf(", symbol = 0x%02x", tree->symbol);
        }
    }

    printf("\n");
    node_print_tree(tree->left, '\\', indentation + 3);
}
