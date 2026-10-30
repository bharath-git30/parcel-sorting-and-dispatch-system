#ifndef QUEUE_H
#define QUEUE_H

#include "parcel.h"
#include <stdbool.h>

/* Singly linked list node for FIFO Queue */
typedef struct QueueNode {
    Parcel* parcel;
    struct QueueNode* next;
} QueueNode;

/* FIFO Queue structure with optional capacity constraint */
typedef struct Queue {
    QueueNode* front;
    QueueNode* rear;
    int size;
    int capacity; /* <= 0 indicates unbounded */
} Queue;

Queue* queue_create(int capacity);
void queue_free(Queue* q);

bool queue_enqueue(Queue* q, Parcel* parcel);
Parcel* queue_dequeue(Queue* q);
Parcel* queue_peek(const Queue* q);

bool queue_is_empty(const Queue* q);
bool queue_is_full(const Queue* q);
int queue_size(const Queue* q);

void queue_print(const Queue* q);

#endif /* QUEUE_H */
