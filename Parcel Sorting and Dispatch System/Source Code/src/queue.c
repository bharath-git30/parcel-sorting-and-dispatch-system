#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

Queue* queue_create(int capacity) {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    if (!q) {
        fprintf(stderr, "Error: Memory allocation failed for Queue.\n");
        return NULL;
    }
    q->front = NULL;
    q->rear = NULL;
    q->size = 0;
    q->capacity = capacity;
    return q;
}

void queue_free(Queue* q) {
    QueueNode* curr;
    if (!q) return;
    curr = q->front;
    while (curr) {
        QueueNode* next = curr->next;
        free(curr);
        curr = next;
    }
    free(q);
}

bool queue_is_empty(const Queue* q) {
    if (!q) return true;
    return (q->size == 0 || q->front == NULL);
}

bool queue_is_full(const Queue* q) {
    if (!q) return false;
    if (q->capacity <= 0) return false; /* Unbounded queue */
    return (q->size >= q->capacity);
}

int queue_size(const Queue* q) {
    if (!q) return 0;
    return q->size;
}

bool queue_enqueue(Queue* q, Parcel* parcel) {
    QueueNode* newNode;
    if (!q || !parcel) return false;
    if (queue_is_full(q)) {
        return false; /* Overflow condition */
    }

    newNode = (QueueNode*)malloc(sizeof(QueueNode));
    if (!newNode) {
        fprintf(stderr, "Error: Memory allocation failed for QueueNode.\n");
        return false;
    }
    newNode->parcel = parcel;
    newNode->next = NULL;

    if (q->rear == NULL) {
        q->front = newNode;
        q->rear = newNode;
    } else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
    q->size++;
    return true;
}

Parcel* queue_dequeue(Queue* q) {
    QueueNode* temp;
    Parcel* parcel;
    if (!q || queue_is_empty(q)) return NULL;

    temp = q->front;
    parcel = temp->parcel;

    q->front = q->front->next;
    if (q->front == NULL) {
        q->rear = NULL;
    }
    free(temp);
    q->size--;
    return parcel;
}

Parcel* queue_peek(const Queue* q) {
    if (!q || queue_is_empty(q)) return NULL;
    return q->front->parcel;
}

void queue_print(const Queue* q) {
    QueueNode* curr;
    int idx;
    if (!q || queue_is_empty(q)) {
        printf("Arrival queue is empty.\n");
        return;
    }
    printf("Arrival Queue (Size: %d / %d) [FIFO - Waiting for sorting]:\n", q->size, q->capacity);
    curr = q->front;
    idx = 1;
    while (curr) {
        printf("  %d. ", idx++);
        parcel_print(curr->parcel);
        curr = curr->next;
    }
}
