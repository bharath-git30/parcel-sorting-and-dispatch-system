#ifndef STACK_H
#define STACK_H

#include "parcel.h"
#include <stdbool.h>

/* Singly linked list node for LIFO Stack */
typedef struct StackNode {
    Parcel* parcel;
    struct StackNode* next;
} StackNode;

/* LIFO Stack structure */
typedef struct Stack {
    StackNode* top;
    int size;
} Stack;

Stack* stack_create(void);
void stack_free(Stack* s);

bool stack_push(Stack* s, Parcel* parcel);
Parcel* stack_pop(Stack* s);
Parcel* stack_peek(const Stack* s);

bool stack_is_empty(const Stack* s);
int stack_size(const Stack* s);

void stack_print(const Stack* s);

#endif /* STACK_H */
