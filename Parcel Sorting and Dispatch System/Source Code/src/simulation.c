#include "simulation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_MSC_VER) && _MSC_VER < 1900
#define snprintf _snprintf
#endif

static int find_city_index(const Simulation* sim, const char* city) {
    int i;
    for (i = 0; i < sim->stats.city_count; i++) {
        if (strcmp(sim->stats.city_stats[i].city, city) == 0) {
            return i;
        }
    }
    return -1;
}

Simulation* sim_create(const char* config_path) {
    Simulation* sim;
    int i;

    sim = (Simulation*)malloc(sizeof(Simulation));
    if (!sim) {
        fprintf(stderr, "Error: Memory allocation failed for Simulation.\n");
        return NULL;
    }
    memset(sim, 0, sizeof(Simulation));

    /* Load Configuration with strict validation */
    if (!config_load(&sim->config, config_path)) {
        printf("[Notice] Proceeding with standard default configuration.\n");
    }

    /* Seed RNG for reproducible dispatch failure simulations */
    if (sim->config.random_seed != 0) {
        srand(sim->config.random_seed);
    } else {
        srand((unsigned int)time(NULL));
    }

    /* Initialize the 5 Core Data Structures */
    sim->arrival_buffer = queue_create(sim->config.queue_capacity);
    sim->return_stack = stack_create();
    sim->destination_sorter = bst_create();
    sim->parcel_tracker = hash_create(DEFAULT_HASH_CAPACITY);
    sim->terminal_rotator = rotator_create();

    /* Populate BST nodes and Circular List terminals for unique cities */
    for (i = 0; i < sim->config.city_count; i++) {
        bst_insert_city(sim->destination_sorter, sim->config.cities[i]);
        rotator_add_terminal(sim->terminal_rotator, sim->config.cities[i]);

        strncpy(sim->stats.city_stats[i].city, sim->config.cities[i], MAX_CITY_LEN - 1);
        sim->stats.city_stats[i].city[MAX_CITY_LEN - 1] = '\0';
        sim->stats.city_stats[i].registered = 0;
        sim->stats.city_stats[i].accepted = 0;
        sim->stats.city_stats[i].dispatched = 0;
        sim->stats.city_stats[i].misrouted = 0;
    }
    sim->stats.city_count = sim->config.city_count;

    sim->current_tick = 0;         /* Starts at tick 0 as specified */
    sim->parcel_counter = 1000;    /* First parcel assigned P1001 */

    /* Open simulation activity log */
    sim->log_file = fopen("log.txt", "w");
    if (sim->log_file) {
        fprintf(sim->log_file, "================================================================================\n");
        fprintf(sim->log_file, "               PARCEL SORTING AND DISPATCHING SIMULATION LOG                    \n");
        fprintf(sim->log_file, "================================================================================\n");
        fprintf(sim->log_file, "Config: CAPACITY=%d, ROTATION_INTERVAL=%d, MISROUTING_RATE=%.2f\n\n",
                sim->config.queue_capacity,
                sim->config.terminal_rotation_interval,
                sim->config.misrouting_rate);
        fflush(sim->log_file);
    }

    return sim;
}

void sim_free(Simulation* sim) {
    if (!sim) return;

    if (sim->log_file) {
        fclose(sim->log_file);
        sim->log_file = NULL;
    }

    if (sim->arrival_buffer) {
        queue_free(sim->arrival_buffer);
        sim->arrival_buffer = NULL;
    }
    if (sim->return_stack) {
        stack_free(sim->return_stack);
        sim->return_stack = NULL;
    }
    if (sim->destination_sorter) {
        bst_free(sim->destination_sorter);
        sim->destination_sorter = NULL;
    }
    if (sim->terminal_rotator) {
        rotator_free(sim->terminal_rotator);
        sim->terminal_rotator = NULL;
    }

    /* Hash Table holds master ownership of all allocated Parcel structs */
    if (sim->parcel_tracker) {
        hash_free(sim->parcel_tracker, true);
        sim->parcel_tracker = NULL;
    }

    free(sim);
}

int sim_get_city_queue_count(const Simulation* sim) {
    if (!sim || !sim->destination_sorter) return 0;
    return bst_total_parcels(sim->destination_sorter);
}

int sim_get_pending_count(const Simulation* sim) {
    int in_arrival;
    int in_bst;
    int in_stack;
    if (!sim) return 0;
    in_arrival = queue_size(sim->arrival_buffer);
    in_bst = sim_get_city_queue_count(sim);
    in_stack = stack_size(sim->return_stack);
    return in_arrival + in_bst + in_stack;
}

bool sim_add_parcel(Simulation* sim, const char* destination, Priority priority, ParcelSize size, char* out_id, int out_id_len) {
    char id[MAX_ID_LEN];
    int c_idx;
    Parcel* p;

    if (!sim || !destination) return false;

    sim->parcel_counter++;
    snprintf(id, sizeof(id), "P%04d", sim->parcel_counter);
    if (out_id && out_id_len > 0) {
        strncpy(out_id, id, out_id_len - 1);
        out_id[out_id_len - 1] = '\0';
    }

    c_idx = find_city_index(sim, destination);
    sim->stats.total_parcels_registered++;
    if (c_idx >= 0) {
        sim->stats.city_stats[c_idx].registered++;
    }

    /* Record arrival at the current simulation tick */
    p = parcel_create(id, destination, priority, size, sim->current_tick);
    if (!p) {
        fprintf(stderr, "Error: Memory allocation failed for Parcel.\n");
        return false;
    }

    /* Register parcel in Hash Table (ParcelTracker) */
    if (!hash_insert(sim->parcel_tracker, p)) {
        fprintf(stderr, "Error: Failed to register parcel in Hash Table.\n");
        parcel_free(p);
        return false;
    }

    /* Check Arrival Queue capacity */
    if (queue_enqueue(sim->arrival_buffer, p)) {
        p->status = STATUS_WAITING_SORTING;
        sim->stats.total_parcels_accepted++;
        if (c_idx >= 0) {
            sim->stats.city_stats[c_idx].accepted++;
        }
        if (sim->log_file) {
            fprintf(sim->log_file, "[MANUAL ADD] Parcel %s to %s (Pri: %s, Sz: %s) -> Enqueued ArrivalBuffer (Tick %d)\n",
                    id, destination, parcel_priority_to_string(priority), parcel_size_to_string(size), sim->current_tick);
            fflush(sim->log_file);
        }
        return true;
    } else {
        /* Arrival Queue is full: reject parcel */
        p->status = STATUS_REJECTED_OVERFLOW;
        sim->stats.total_parcels_rejected++;
        if (sim->log_file) {
            fprintf(sim->log_file, "[OVERFLOW] Parcel %s to %s REJECTED: ArrivalBuffer full (Tick %d)\n",
                    id, destination, sim->current_tick);
            fflush(sim->log_file);
        }
        return false;
    }
}

void sim_process_tick(Simulation* sim) {
    const char* active_terminal;
    int active_term_id;
    bool rotated = false;
    const char* prev_terminal;

    char reprocessed_str[256] = "None";
    char sorted_str[256] = "None";
    char dispatched_str[256] = "None";
    char returned_str[256] = "None";

    int reprocessed_count = 0;
    int sorted_count = 0;
    int dispatched_count = 0;
    int returned_count = 0;
    int attempt;
    int total_waiting;

    if (!sim) return;

    /* 1. Advance the simulation tick by exactly one */
    sim->current_tick++;
    sim->stats.total_ticks = sim->current_tick;

    /* 2. Terminal Rotation:
     * For interval 5: Chennai serves ticks 1-5, Madurai ticks 6-10, etc.
     * Rotation occurs when entering tick 6, tick 11, etc.
     */
    prev_terminal = rotator_get_current(sim->terminal_rotator);
    if (sim->current_tick > 1 && (sim->current_tick - 1) % sim->config.terminal_rotation_interval == 0) {
        rotator_advance(sim->terminal_rotator);
        rotated = true;
    }
    active_terminal = rotator_get_current(sim->terminal_rotator);
    active_term_id = rotator_get_current_id(sim->terminal_rotator);

    /* 3. Reprocess returned parcels from ReturnStack (LIFO) */
    while (!stack_is_empty(sim->return_stack) && reprocessed_count < sim->config.reprocess_per_tick) {
        Parcel* p = stack_pop(sim->return_stack);
        if (!p) break;

        bst_insert_parcel(sim->destination_sorter, p);
        p->status = STATUS_WAITING_DISPATCH;
        sim->stats.total_reprocessed++;

        if (reprocessed_count == 0) {
            snprintf(reprocessed_str, sizeof(reprocessed_str), "%s", p->id);
        } else {
            size_t len = strlen(reprocessed_str);
            snprintf(reprocessed_str + len, sizeof(reprocessed_str) - len, ", %s", p->id);
        }
        reprocessed_count++;

        if (sim->log_file) {
            fprintf(sim->log_file, "[TICK %03d][REPROCESS] Popped %s (LIFO) -> Re-enqueued in %s city queue\n",
                    sim->current_tick, p->id, p->destination);
        }
    }

    /* 4. Sort parcels from Arrival Queue into BST destination queues */
    while (!queue_is_empty(sim->arrival_buffer) && sorted_count < sim->config.sort_per_tick) {
        Parcel* p = queue_dequeue(sim->arrival_buffer);
        if (!p) break;

        bst_insert_parcel(sim->destination_sorter, p);
        p->status = STATUS_WAITING_DISPATCH;

        if (sorted_count == 0) {
            snprintf(sorted_str, sizeof(sorted_str), "%s", p->id);
        } else {
            size_t len = strlen(sorted_str);
            snprintf(sorted_str + len, sizeof(sorted_str) - len, ", %s", p->id);
        }
        sorted_count++;

        if (sim->log_file) {
            fprintf(sim->log_file, "[TICK %03d][SORT] Dequeued %s -> Sorted into %s city queue\n",
                    sim->current_tick, p->id, p->destination);
        }
    }

    /* 5. Attempt dispatch for the currently active destination */
    for (attempt = 0; attempt < sim->config.dispatch_per_tick; attempt++) {
        Parcel* p = bst_retrieve_parcel(sim->destination_sorter, active_terminal);
        double prob;
        int c_idx;

        if (!p) {
            /* No more parcels waiting in this city's queue */
            break;
        }

        /* Check simulated dispatch misrouting */
        prob = (double)rand() / (double)RAND_MAX;
        c_idx = find_city_index(sim, p->destination);

        if (prob < sim->config.misrouting_rate) {
            /* Dispatch failure: push to ReturnStack (LIFO) */
            p->return_count++;
            p->status = STATUS_RETURNED;
            stack_push(sim->return_stack, p);

            sim->stats.total_misroute_events++;
            if (c_idx >= 0) {
                sim->stats.city_stats[c_idx].misrouted++;
            }

            if (returned_count == 0) {
                snprintf(returned_str, sizeof(returned_str), "%s", p->id);
            } else {
                size_t len = strlen(returned_str);
                snprintf(returned_str + len, sizeof(returned_str) - len, ", %s", p->id);
            }
            returned_count++;

            if (sim->log_file) {
                fprintf(sim->log_file, "[TICK %03d][MISROUTE] %s to %s failed dispatch -> Pushed to ReturnStack (Return #%d)\n",
                        sim->current_tick, p->id, p->destination, p->return_count);
            }
        } else {
            /* Dispatch success */
            int delay;
            p->status = STATUS_DISPATCHED;
            p->dispatch_tick = sim->current_tick;
            delay = p->dispatch_tick - p->arrival_tick;
            sim->stats.total_delay_ticks += delay;
            sim->stats.total_parcels_dispatched++;

            if (c_idx >= 0) {
                sim->stats.city_stats[c_idx].dispatched++;
            }

            if (dispatched_count == 0) {
                snprintf(dispatched_str, sizeof(dispatched_str), "%s", p->id);
            } else {
                size_t len = strlen(dispatched_str);
                snprintf(dispatched_str + len, sizeof(dispatched_str) - len, ", %s", p->id);
            }
            dispatched_count++;

            if (sim->log_file) {
                fprintf(sim->log_file, "[TICK %03d][DISPATCH] %s to %s successfully dispatched! (Transit delay: %d ticks)\n",
                        sim->current_tick, p->id, p->destination, delay);
            }
        }
    }

    if (sim->log_file) {
        fflush(sim->log_file);
    }

    /* 6. Display concise tick summary */
    total_waiting = sim_get_pending_count(sim);

    printf("\n------------------------------------------------------------\n");
    printf("Tick: %d\n", sim->current_tick);
    printf("Active terminal: %s (Terminal %d)%s\n",
           active_terminal, active_term_id,
           rotated ? " [Rotated this tick]" : "");
    if (reprocessed_count > 0) {
        printf("Reprocessed: %s (LIFO from Return Stack -> City Queue)\n", reprocessed_str);
    } else {
        printf("Reprocessed: None\n");
    }
    printf("Sorted: %s\n", sorted_str);
    printf("Dispatched: %s\n", dispatched_str);
    if (returned_count > 0) {
        printf("Returned: %s (Simulated failure -> Pushed to Return Stack)\n", returned_str);
    } else {
        printf("Returned: None\n");
    }
    if (dispatched_count == 0 && returned_count == 0) {
        printf("Dispatch note: Terminal '%s' had no parcels ready for dispatch.\n", active_terminal);
    }
    printf("Still waiting: %d parcel(s) (%d in arrival queue, %d in city queues, %d in return stack)\n",
           total_waiting,
           queue_size(sim->arrival_buffer),
           sim_get_city_queue_count(sim),
           stack_size(sim->return_stack));
    printf("------------------------------------------------------------\n");
}
