#include "pq.h"

#include "node.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct ListElement ListElement;
struct ListElement {
    Node *tree;
    ListElement *next;
};

struct PriorityQueue {
    ListElement *list;
};

PriorityQueue *pq_create(void) {
    PriorityQueue *queue = calloc(1, sizeof(PriorityQueue));
    if (queue == NULL) {
        // Handle memory allocation error
        return NULL;
    }
    return queue;
}

void pq_free(PriorityQueue **q) {
    if (q == NULL || *q == NULL) {
        return;
    }

    ListElement *current = (*q)->list;
    while (current != NULL) {
        ListElement *next = current->next;
        free(current);
        current = next;
    }

    free(*q);
    *q = NULL;
}

bool pq_is_empty(PriorityQueue *q) {
    return q->list == NULL;
}

bool pq_size_is_1(PriorityQueue *q) {
    return q->list != NULL && q->list->next == NULL;
}

void enqueue(PriorityQueue *q, Node *tree) {
    ListElement *e = malloc(sizeof(ListElement));
    if (e == NULL) {
        // Handle memory allocation error
        return;
    }
    e->tree = tree;

    if (q->list == NULL || tree->weight < q->list->tree->weight) {
        // Insert at the beginning of the queue
        e->next = q->list;
        q->list = e;
    } else {
        ListElement *current = q->list;
        while (current->next != NULL && tree->weight >= current->next->tree->weight) {
            current = current->next;
        }
        e->next = current->next;
        current->next = e;
    }
}

bool dequeue(PriorityQueue *q, Node **tree) {
    if (q->list == NULL) {
        // Queue is empty
        return false;
    }

    ListElement *e = q->list;
    q->list = q->list->next;
    *tree = e->tree;
    free(e);

    return true;
}

void pq_print(PriorityQueue *q) {
    assert(q != NULL);

    ListElement *e = q->list;
    int position = 1;

    while (e != NULL) {
        if (position++ == 1) {
            printf("=============================================\n");
        } else {
            printf("---------------------------------------------\n");
        }
        node_print_tree(e->tree, '<', 2);
        e = e->next;
    }

    printf("=============================================\n");
}

bool pq_less_than(Node *n1, Node *n2) {
    if (n1->weight < n2->weight)
        return true;
    if (n1->weight > n2->weight)
        return false;
    return n1->symbol < n2->symbol;
}
