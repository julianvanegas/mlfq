#ifndef QUEUE_H
#define QUEUE_H
#include "process.h"

typedef struct Queue Queue;

Queue* queue_create();
void queue_destroy(Queue* q);
void queue_push(Queue* q, Process* p);
Process* queue_pop(Queue* q);
int queue_is_empty(Queue* q);
Process* queue_peek(Queue* q);
int queue_get_size(Queue* q);

#endif