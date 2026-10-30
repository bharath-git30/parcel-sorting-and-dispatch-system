#ifndef CIRCULAR_LIST_H
#define CIRCULAR_LIST_H

#include "parcel.h"
#include <stdbool.h>

/* Node for Circular Linked List representing dispatch terminals */
typedef struct TerminalNode {
    char city[MAX_CITY_LEN];
    int terminal_id;
    struct TerminalNode* next;
} TerminalNode;

/* Circular Linked List structure */
typedef struct CircularList {
    TerminalNode* current;
    int count;
} CircularList;

CircularList* rotator_create(void);
void rotator_free(CircularList* list);

bool rotator_add_terminal(CircularList* list, const char* city);
const char* rotator_get_current(const CircularList* list);
int rotator_get_current_id(const CircularList* list);
const char* rotator_advance(CircularList* list);
int rotator_count(const CircularList* list);

void rotator_print(const CircularList* list);

#endif /* CIRCULAR_LIST_H */
