#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static void safe_get_string(char* buffer, int max_len) {
    if (fgets(buffer, max_len, stdin) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
    }
}

static void display_main_menu(const Simulation* sim) {
    const char* active_city = rotator_get_current(sim->terminal_rotator);
    printf("\n");
    printf("============================================================\n");
    printf("           PARCEL SORTING AND DISPATCHING SYSTEM            \n");
    printf("============================================================\n");
    printf("Current Tick: %d | Active Terminal: %s\n", sim->current_tick, active_city);
    printf("============================================================\n");
    printf("1. Add a parcel\n");
    printf("2. Process one tick\n");
    printf("3. View arrival queue\n");
    printf("4. View parcels by city\n");
    printf("5. View returned parcels\n");
    printf("6. View active terminal\n");
    printf("7. Search parcel by ID\n");
    printf("8. Exit\n");
    printf("============================================================\n");
    printf("Enter choice (1-8): ");
}

static void handle_add_parcel(Simulation* sim) {
    char input[64];
    int city_choice;
    int pri_choice;
    int size_choice;
    int i;
    char assigned_id[MAX_ID_LEN];
    Priority priority = PRIORITY_MEDIUM;
    ParcelSize psize = SIZE_MEDIUM;
    const char* selected_city;
    bool success;

    printf("\n--- Add a Parcel ---\n");
    printf("Select Destination City:\n");
    for (i = 0; i < sim->config.city_count; i++) {
        printf("  %d. %s\n", i + 1, sim->config.cities[i]);
    }
    printf("Enter choice (1-%d): ", sim->config.city_count);
    safe_get_string(input, sizeof(input));
    city_choice = atoi(input);

    if (city_choice < 1 || city_choice > sim->config.city_count) {
        printf("[Invalid Selection] Please choose a valid destination number (1-%d).\n", sim->config.city_count);
        return;
    }
    selected_city = sim->config.cities[city_choice - 1];

    printf("\nSelect Priority (Informational - does not alter dispatch order):\n");
    printf("  1. Low\n");
    printf("  2. Medium\n");
    printf("  3. High\n");
    printf("Enter choice (1-3, default 2): ");
    safe_get_string(input, sizeof(input));
    pri_choice = atoi(input);
    if (pri_choice == 1) priority = PRIORITY_LOW;
    else if (pri_choice == 3) priority = PRIORITY_HIGH;
    else priority = PRIORITY_MEDIUM;

    printf("\nSelect Size (Informational - does not alter capacity):\n");
    printf("  1. Small\n");
    printf("  2. Medium\n");
    printf("  3. Large\n");
    printf("Enter choice (1-3, default 2): ");
    safe_get_string(input, sizeof(input));
    size_choice = atoi(input);
    if (size_choice == 1) psize = SIZE_SMALL;
    else if (size_choice == 3) psize = SIZE_LARGE;
    else psize = SIZE_MEDIUM;

    /* Add parcel: does not advance tick */
    success = sim_add_parcel(sim, selected_city, priority, psize, assigned_id, sizeof(assigned_id));

    if (success) {
        printf("\n[Success] Parcel %s added to Arrival Queue.\n", assigned_id);
        printf("  Destination : %s\n", selected_city);
        printf("  Priority    : %s (Informational)\n", parcel_priority_to_string(priority));
        printf("  Size        : %s (Informational)\n", parcel_size_to_string(psize));
        printf("  Arrival Tick: %d\n", sim->current_tick);
        printf("  Status      : Waiting for sorting\n");
        printf("  Queue Status: %d / %d parcels in arrival queue\n",
               queue_size(sim->arrival_buffer), sim->config.queue_capacity);
    } else {
        printf("\n[Queue Full] Arrival Queue is at full capacity (%d/%d).\n",
               queue_size(sim->arrival_buffer), sim->config.queue_capacity);
        printf("Parcel %s for %s could not be accepted (Status: Rejected).\n",
               assigned_id, selected_city);
    }
}

static void handle_view_active_terminal(const Simulation* sim) {
    int ticks_into_interval;
    int ticks_until_rotate;
    const char* active_city;
    int active_id;

    active_city = rotator_get_current(sim->terminal_rotator);
    active_id = rotator_get_current_id(sim->terminal_rotator);

    printf("\n--- Active Terminal & Rotation Sequence ---\n");
    printf("Current Active City     : %s (Terminal %d)\n", active_city, active_id);
    printf("Rotation Interval       : Every %d ticks\n", sim->config.terminal_rotation_interval);
    printf("Current Simulation Tick : %d\n", sim->current_tick);

    if (sim->current_tick == 0) {
        printf("Next Rotation           : At Tick %d (in %d tick(s))\n",
               sim->config.terminal_rotation_interval + 1,
               sim->config.terminal_rotation_interval + 1);
    } else {
        ticks_into_interval = (sim->current_tick - 1) % sim->config.terminal_rotation_interval;
        ticks_until_rotate = sim->config.terminal_rotation_interval - ticks_into_interval;
        printf("Next Rotation           : In %d tick(s) (at Tick %d)\n",
               ticks_until_rotate, sim->current_tick + ticks_until_rotate);
    }

    printf("\nTerminal Rotation Sequence (Circular Linked List):\n");
    rotator_print(sim->terminal_rotator);
}

static void handle_search_parcel(const Simulation* sim) {
    char input[MAX_ID_LEN];
    Parcel* p;

    printf("\n--- Search Parcel by ID (Hash Table) ---\n");
    printf("Enter Parcel ID to search (e.g. P1001, P1002): ");
    safe_get_string(input, sizeof(input));

    if (strlen(input) == 0) {
        printf("No Parcel ID entered.\n");
        return;
    }

    p = hash_search(sim->parcel_tracker, input);
    if (p) {
        printf("\nParcel Record Found in Hash Table:\n");
        printf("  Parcel ID       : %s\n", p->id);
        printf("  Destination City: %s\n", p->destination);
        printf("  Priority        : %s (Informational)\n", parcel_priority_to_string(p->priority));
        printf("  Physical Size   : %s (Informational)\n", parcel_size_to_string(p->size));
        printf("  Arrival Tick    : %d\n", p->arrival_tick);
        if (p->dispatch_tick >= 0) {
            printf("  Dispatch Tick   : %d\n", p->dispatch_tick);
            printf("  Transit Delay   : %d tick(s)\n", p->dispatch_tick - p->arrival_tick);
        } else {
            printf("  Dispatch Tick   : Not yet dispatched\n");
            printf("  Transit Delay   : In transit (currently waiting)\n");
        }
        printf("  Return Count    : %d time(s)\n", p->return_count);
        printf("  Current Status  : %s\n", parcel_status_to_string(p->status));
    } else {
        printf("\n[Not Found] Parcel ID '%s' does not exist in the system.\n", input);
    }
}

void ui_run_interactive(Simulation* sim) {
    char input[64];
    int choice = 0;

    if (!sim) return;

    while (1) {
        display_main_menu(sim);
        if (!fgets(input, sizeof(input), stdin)) break;

        choice = atoi(input);

        switch (choice) {
            case 1:
                handle_add_parcel(sim);
                break;
            case 2:
                sim_process_tick(sim);
                break;
            case 3:
                printf("\n--- Arrival Queue (FIFO Buffer) ---\n");
                queue_print(sim->arrival_buffer);
                break;
            case 4:
                printf("\n--- Parcels Grouped by Destination City (BST) ---\n");
                bst_print_detailed(sim->destination_sorter);
                break;
            case 5:
                printf("\n--- Returned Parcels (LIFO Stack) ---\n");
                stack_print(sim->return_stack);
                break;
            case 6:
                handle_view_active_terminal(sim);
                break;
            case 7:
                handle_search_parcel(sim);
                break;
            case 8:
                printf("\nExiting application. Releasing all allocated memory...\n");
                return;
            default:
                printf("\n[Invalid Selection] Please choose a number between 1 and 8.\n");
                break;
        }
    }
}
