#include "hashtable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned int hash_function(const char* str, int capacity) {
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return (unsigned int)(hash % capacity);
}

HashTable* hash_create(int initial_capacity) {
    HashTable* table;
    if (initial_capacity <= 0) {
        initial_capacity = DEFAULT_HASH_CAPACITY;
    }
    table = (HashTable*)malloc(sizeof(HashTable));
    if (!table) {
        fprintf(stderr, "Error: Memory allocation failed for HashTable.\n");
        return NULL;
    }
    table->capacity = initial_capacity;
    table->size = 0;
    table->max_load_factor = DEFAULT_MAX_LOAD_FACTOR;
    table->buckets = (HashNode**)calloc(table->capacity, sizeof(HashNode*));
    if (!table->buckets) {
        fprintf(stderr, "Error: Memory allocation failed for HashTable buckets.\n");
        free(table);
        return NULL;
    }
    return table;
}

void hash_free(HashTable* table, bool free_parcels) {
    int i;
    if (!table) return;
    for (i = 0; i < table->capacity; i++) {
        HashNode* curr = table->buckets[i];
        while (curr) {
            HashNode* next = curr->next;
            if (free_parcels && curr->parcel) {
                parcel_free(curr->parcel);
            }
            free(curr);
            curr = next;
        }
    }
    free(table->buckets);
    free(table);
}

float hash_load_factor(const HashTable* table) {
    if (!table || table->capacity <= 0) return 0.0f;
    return (float)table->size / (float)table->capacity;
}

void hash_resize_and_rehash(HashTable* table, int new_capacity) {
    HashNode** new_buckets;
    int i;
    if (!table || new_capacity <= table->size) return;

    new_buckets = (HashNode**)calloc(new_capacity, sizeof(HashNode*));
    if (!new_buckets) {
        fprintf(stderr, "Warning: Failed to allocate memory for HashTable resize.\n");
        return;
    }

    /* Rehash all existing nodes into new buckets */
    for (i = 0; i < table->capacity; i++) {
        HashNode* curr = table->buckets[i];
        while (curr) {
            HashNode* next = curr->next;
            unsigned int idx = hash_function(curr->key, new_capacity);
            curr->next = new_buckets[idx];
            new_buckets[idx] = curr;
            curr = next;
        }
    }

    free(table->buckets);
    table->buckets = new_buckets;
    table->capacity = new_capacity;
}

bool hash_insert(HashTable* table, Parcel* parcel) {
    unsigned int idx;
    HashNode* curr;
    HashNode* newNode;
    if (!table || !parcel) return false;

    /* Check if resizing is needed */
    if (hash_load_factor(table) >= table->max_load_factor) {
        hash_resize_and_rehash(table, table->capacity * 2);
    }

    idx = hash_function(parcel->id, table->capacity);
    curr = table->buckets[idx];

    /* Check if parcel already exists in table */
    while (curr) {
        if (strcmp(curr->key, parcel->id) == 0) {
            curr->parcel = parcel; /* Update reference */
            return true;
        }
        curr = curr->next;
    }

    /* Allocate new node and prepend to chain */
    newNode = (HashNode*)malloc(sizeof(HashNode));
    if (!newNode) {
        fprintf(stderr, "Error: Memory allocation failed for HashNode.\n");
        return false;
    }
    strncpy(newNode->key, parcel->id, sizeof(newNode->key) - 1);
    newNode->key[sizeof(newNode->key) - 1] = '\0';
    newNode->parcel = parcel;
    newNode->next = table->buckets[idx];
    table->buckets[idx] = newNode;
    table->size++;
    return true;
}

Parcel* hash_search(const HashTable* table, const char* parcel_id) {
    unsigned int idx;
    HashNode* curr;
    if (!table || !parcel_id) return NULL;
    idx = hash_function(parcel_id, table->capacity);
    curr = table->buckets[idx];
    while (curr) {
        if (strcmp(curr->key, parcel_id) == 0) {
            return curr->parcel;
        }
        curr = curr->next;
    }
    return NULL;
}

bool hash_exists(const HashTable* table, const char* parcel_id) {
    return (hash_search(table, parcel_id) != NULL);
}

bool hash_update_status(HashTable* table, const char* parcel_id, ParcelStatus new_status) {
    Parcel* p = hash_search(table, parcel_id);
    if (!p) return false;
    p->status = new_status;
    return true;
}

bool hash_update_return_count(HashTable* table, const char* parcel_id) {
    Parcel* p = hash_search(table, parcel_id);
    if (!p) return false;
    p->return_count++;
    return true;
}

void hash_print_stats(const HashTable* table) {
    int used_buckets = 0;
    int max_chain = 0;
    int i;
    if (!table) {
        printf("  [Empty Hash Table]\n");
        return;
    }
    for (i = 0; i < table->capacity; i++) {
        if (table->buckets[i]) {
            int chain_len = 0;
            HashNode* curr = table->buckets[i];
            used_buckets++;
            while (curr) {
                chain_len++;
                curr = curr->next;
            }
            if (chain_len > max_chain) {
                max_chain = chain_len;
            }
        }
    }
    printf("  Hash Table (ParcelTracker) Stats:\n");
    printf("    Total Tracked Items : %d\n", table->size);
    printf("    Bucket Capacity     : %d\n", table->capacity);
    printf("    Occupied Buckets    : %d (%.1f%% utilization)\n",
           used_buckets, (used_buckets * 100.0f) / table->capacity);
    printf("    Longest Collision   : %d items\n", max_chain);
    printf("    Current Load Factor : %.3f (Threshold: %.2f)\n",
           hash_load_factor(table), table->max_load_factor);
}

void hash_dump_all(const HashTable* table) {
    int count = 1;
    int i;
    if (!table || table->size == 0) {
        printf("  [No parcels currently tracked]\n");
        return;
    }
    printf("  Tracked Parcels (Total: %d):\n", table->size);
    for (i = 0; i < table->capacity; i++) {
        HashNode* curr = table->buckets[i];
        while (curr) {
            printf("    %3d. ", count++);
            parcel_print(curr->parcel);
            curr = curr->next;
        }
    }
}
