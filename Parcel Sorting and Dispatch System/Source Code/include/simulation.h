#ifndef SIMULATION_H
#define SIMULATION_H

#include "config.h"
#include "parcel.h"
#include "queue.h"
#include "stack.h"
#include "bst.h"
#include "hashtable.h"
#include "circular_list.h"
#include <stdio.h>
#include <stdbool.h>

typedef struct CityStats {
    char city[MAX_CITY_LEN];
    int registered;
    int accepted;
    int dispatched;
    int misrouted;
} CityStats;

typedef struct SimStats {
    int total_ticks;
    int total_parcels_registered;
    int total_parcels_accepted;
    int total_parcels_rejected;
    int total_parcels_dispatched;
    int total_misroute_events;
    int total_reprocessed;
    long long total_delay_ticks;
    CityStats city_stats[MAX_CITIES];
    int city_count;
} SimStats;

typedef struct Simulation {
    Config config;
    int current_tick;
    int parcel_counter;

    /* The 5 Core Data Structures */
    Queue* arrival_buffer;           /* 1. Singly Linked List FIFO Queue */
    Stack* return_stack;             /* 2. Singly Linked List LIFO Stack */
    BST* destination_sorter;         /* 3. Binary Search Tree with per-city Queues */
    HashTable* parcel_tracker;       /* 4. Hash Table with Chaining */
    CircularList* terminal_rotator;  /* 5. Circular Linked List */

    SimStats stats;
    FILE* log_file;
} Simulation;

Simulation* sim_create(const char* config_path);
void sim_free(Simulation* sim);

/* Manual parcel entry */
bool sim_add_parcel(Simulation* sim, const char* destination, Priority priority, ParcelSize size, char* out_id, int out_id_len);

/* User-controlled single tick process */
void sim_process_tick(Simulation* sim);

/* Helper queries */
int sim_get_pending_count(const Simulation* sim);
int sim_get_city_queue_count(const Simulation* sim);

#endif /* SIMULATION_H */
