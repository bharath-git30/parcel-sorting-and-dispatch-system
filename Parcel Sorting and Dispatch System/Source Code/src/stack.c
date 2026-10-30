#include "stack.h"
#include <stdio.h>
#include <stdlib.h>

Stack* stack_create(void) {
    Stack* s = (Stack*)malloc(sizeof(Stack));
    if (!s) {
        fprintf(stderr, "Error: Memory allocation failed for Stack.\n");
        return NULL;
    }
    s->top = NULL;
    s->size = 0;
    return s;
}

void stack_free(Stack* s) {
    StackNode* curr;
    if (!s) return;
    curr = s->top;
    while (curr) {
        StackNode* next = curr->next;
        free(curr);
        curr = next;
    }
    free(s);
}

bool stack_is_empty(const Stack* s) {
    if (!s) return true;
    return (s->size == 0 || s->top == NULL);
}

int stack_size(const Stack* s) {
    if (!s) return 0;
    return s->size;
}

bool stack_push(Stack* s, Parcel* parcel) {
    StackNode* newNode;
    if (!s || !parcel) return false;
    newNode = (StackNode*)malloc(sizeof(StackNode));
    if (!newNode) {
        fprintf(stderr, "Error: Memory allocation failed for StackNode.\n");
        return false;
    }
    newNode->parcel = parcel;
    newNode->next = s->top;
    s->top = newNode;
    s->size++;
    return true;
}

Parcel* stack_pop(Stack* s) {
    StackNode* temp;
    Parcel* parcel;
    if (!s || stack_is_empty(s)) return NULL;
    temp = s->top;
    parcel = temp->parcel;
    s->top = s->top->next;
    free(temp);
    s->size--;
    return parcel;
}

Parcel* stack_peek(const Stack* s) {
    if (!s || stack_is_empty(s)) return NULL;
    return s->top->parcel;
}

void stack_print(const Stack* s) {
    StackNode* curr;
    int depth;
    if (!s || stack_is_empty(s)) {
        printf("Return stack is empty (no returned parcels).\n");
        return;
    }
    printf("Return Stack (Size: %d) [LIFO - Waiting for reprocessing]:\n", s->size);
    curr = s->top;
    depth = 1;
    while (curr) {
        printf("  [Top #%d] ", depth++);
        parcel_print(curr->parcel);
        curr = curr->next;
    }
}
