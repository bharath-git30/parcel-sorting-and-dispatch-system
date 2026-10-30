#include "parcel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Parcel* parcel_create(const char* id, const char* destination, Priority priority, ParcelSize size, int arrival_tick) {
    Parcel* p = (Parcel*)malloc(sizeof(Parcel));
    if (!p) {
        fprintf(stderr, "Error: Memory allocation failed for Parcel.\n");
        return NULL;
    }
    strncpy(p->id, id, sizeof(p->id) - 1);
    p->id[sizeof(p->id) - 1] = '\0';

    strncpy(p->destination, destination, sizeof(p->destination) - 1);
    p->destination[sizeof(p->destination) - 1] = '\0';

    p->priority = priority;
    p->size = size;
    p->arrival_tick = arrival_tick;
    p->dispatch_tick = -1;
    p->status = STATUS_WAITING_SORTING;
    p->return_count = 0;
    return p;
}

void parcel_free(Parcel* parcel) {
    if (parcel) {
        free(parcel);
    }
}

const char* parcel_priority_to_string(Priority priority) {
    switch (priority) {
        case PRIORITY_LOW:    return "Low";
        case PRIORITY_MEDIUM: return "Medium";
        case PRIORITY_HIGH:   return "High";
        default:              return "Normal";
    }
}

const char* parcel_size_to_string(ParcelSize size) {
    switch (size) {
        case SIZE_SMALL:  return "Small";
        case SIZE_MEDIUM: return "Medium";
        case SIZE_LARGE:  return "Large";
        default:          return "Standard";
    }
}

const char* parcel_status_to_string(ParcelStatus status) {
    switch (status) {
        case STATUS_WAITING_SORTING:    return "Waiting for sorting";
        case STATUS_WAITING_DISPATCH:   return "Waiting for dispatch";
        case STATUS_DISPATCHED:         return "Dispatched";
        case STATUS_RETURNED:           return "Returned for reprocessing";
        case STATUS_REJECTED_OVERFLOW:  return "Rejected (Queue Full)";
        default:                        return "Unknown";
    }
}

void parcel_print(const Parcel* parcel) {
    if (!parcel) {
        printf("[Null Parcel]\n");
        return;
    }
    printf("[%s] Dest: %-10s | Pri: %-6s | Size: %-6s | Arr: Tick %d | Ret: %d | Status: %s\n",
           parcel->id, parcel->destination,
           parcel_priority_to_string(parcel->priority),
           parcel_size_to_string(parcel->size),
           parcel->arrival_tick, parcel->return_count,
           parcel_status_to_string(parcel->status));
}
