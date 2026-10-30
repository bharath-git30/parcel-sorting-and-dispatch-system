#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "parcel.h"
#include <stdbool.h>

#define DEFAULT_HASH_CAPACITY 64
#define DEFAULT_MAX_LOAD_FACTOR 0.75f

/* Linked list node for collision chaining */
typedef struct HashNode {
    char key[MAX_ID_LEN];
    Parcel* parcel;
    struct HashNode* next;
} HashNode;

/* Hash Table with chaining and dynamic rehashing */
typedef struct HashTable {
    HashNode** buckets;
    int capacity;
    int size;
    float max_load_factor;
} HashTable;

HashTable* hash_create(int initial_capacity);
void hash_free(HashTable* table, bool free_parcels);

bool hash_insert(HashTable* table, Parcel* parcel);
Parcel* hash_search(const HashTable* table, const char* parcel_id);
bool hash_exists(const HashTable* table, const char* parcel_id);

bool hash_update_status(HashTable* table, const char* parcel_id, ParcelStatus new_status);
bool hash_update_return_count(HashTable* table, const char* parcel_id);

float hash_load_factor(const HashTable* table);
void hash_resize_and_rehash(HashTable* table, int new_capacity);

void hash_print_stats(const HashTable* table);
void hash_dump_all(const HashTable* table);

#endif /* HASHTABLE_H */
