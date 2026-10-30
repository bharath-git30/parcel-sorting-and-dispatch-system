#include "circular_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CircularList* rotator_create(void) {
    CircularList* list = (CircularList*)malloc(sizeof(CircularList));
    if (!list) {
        fprintf(stderr, "Error: Memory allocation failed for CircularList.\n");
        return NULL;
    }
    list->current = NULL;
    list->count = 0;
    return list;
}

void rotator_free(CircularList* list) {
    if (!list) return;
    if (list->current) {
        TerminalNode* curr = list->current;
        TerminalNode* nextNode = NULL;
        int n = list->count;
        int i;
        for (i = 0; i < n; i++) {
            nextNode = curr->next;
            free(curr);
            curr = nextNode;
        }
    }
    free(list);
}

bool rotator_add_terminal(CircularList* list, const char* city) {
    TerminalNode* newNode;
    TerminalNode* tail;
    if (!list || !city) return false;

    newNode = (TerminalNode*)malloc(sizeof(TerminalNode));
    if (!newNode) {
        fprintf(stderr, "Error: Memory allocation failed for TerminalNode.\n");
        return false;
    }
    strncpy(newNode->city, city, sizeof(newNode->city) - 1);
    newNode->city[sizeof(newNode->city) - 1] = '\0';
    newNode->terminal_id = list->count + 1;

    if (list->current == NULL) {
        newNode->next = newNode;
        list->current = newNode;
    } else {
        tail = list->current;
        while (tail->next != list->current) {
            tail = tail->next;
        }
        tail->next = newNode;
        newNode->next = list->current;
    }
    list->count++;
    return true;
}

const char* rotator_get_current(const CircularList* list) {
    if (!list || !list->current) return "None";
    return list->current->city;
}

int rotator_get_current_id(const CircularList* list) {
    if (!list || !list->current) return 0;
    return list->current->terminal_id;
}

const char* rotator_advance(CircularList* list) {
    if (!list || !list->current) return "None";
    list->current = list->current->next;
    return list->current->city;
}

int rotator_count(const CircularList* list) {
    if (!list) return 0;
    return list->count;
}

void rotator_print(const CircularList* list) {
    TerminalNode* curr;
    int i;
    if (!list || !list->current || list->count == 0) {
        printf("Terminal rotator is empty.\n");
        return;
    }
    printf("Dispatch Terminals (Total: %d in repeating cycle):\n", list->count);
    curr = list->current;
    for (i = 0; i < list->count; i++) {
        const char* indicator = (curr == list->current) ? " <-- [ACTIVE TERMINAL]" : "";
        printf("  Terminal %d: %-15s%s\n", curr->terminal_id, curr->city, indicator);
        curr = curr->next;
    }
    printf("  (Rotation order returns to Terminal 1 after Terminal %d)\n", list->count);
}
