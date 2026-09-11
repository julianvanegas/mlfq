#include "queue.h"
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    Process* process;
    struct Node* next;
} Node;

struct Queue {
    Node* head;
    Node* tail;
};

Queue* queue_create() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    if (q) { q->head = NULL; q->tail = NULL; }
    return q;
}

void queue_destroy(Queue* q) {
    while (!queue_is_empty(q)) queue_pop(q);
    free(q);
}

void queue_push(Queue* q, Process* p) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->process = p;
    node->next = NULL;
    if (queue_is_empty(q)) {
        q->head = node;
    } else {
        q->tail->next = node;
    }
    q->tail = node;
}

Process* queue_pop(Queue* q) {
    if (queue_is_empty(q)) return NULL;
    Node* temp = q->head;
    Process* p = temp->process;
    q->head = q->head->next;
    if (q->head == NULL) q->tail = NULL;
    free(temp);
    return p;
}

int queue_is_empty(Queue* q) {
    return q->head == NULL;
}

Process* queue_peek(Queue* q) {
    return queue_is_empty(q) ? NULL : q->head->process;
}

int queue_get_size(Queue* q) {
    int count = 0;
    Node* current = q->head;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}