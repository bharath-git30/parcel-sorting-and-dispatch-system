#ifndef PARCEL_H
#define PARCEL_H

#include <stdbool.h>

#define MAX_ID_LEN 32
#define MAX_CITY_LEN 64

/* Priority is informational (does not affect dispatch order) */
typedef enum {
    PRIORITY_LOW = 1,
    PRIORITY_MEDIUM = 2,
    PRIORITY_HIGH = 3
} Priority;

/* Size is informational (does not affect capacity) */
typedef enum {
    SIZE_SMALL = 1,
    SIZE_MEDIUM = 2,
    SIZE_LARGE = 3
} ParcelSize;

/* Clear parcel status transitions */
typedef enum {
    STATUS_WAITING_SORTING = 0,    /* Waiting in Arrival Queue */
    STATUS_WAITING_DISPATCH,       /* Waiting in Destination City Queue (BST) */
    STATUS_DISPATCHED,             /* Successfully Dispatched */
    STATUS_RETURNED,               /* Failed dispatch; waiting in Return Stack */
    STATUS_REJECTED_OVERFLOW       /* Arrival Queue was full when added */
} ParcelStatus;

typedef struct Parcel {
    char id[MAX_ID_LEN];
    char destination[MAX_CITY_LEN];
    Priority priority;
    ParcelSize size;
    int arrival_tick;
    int dispatch_tick;
    ParcelStatus status;
    int return_count;
} Parcel;

/* Parcel lifecycle and utility functions */
Parcel* parcel_create(const char* id, const char* destination, Priority priority, ParcelSize size, int arrival_tick);
void parcel_free(Parcel* parcel);

const char* parcel_priority_to_string(Priority priority);
const char* parcel_size_to_string(ParcelSize size);
const char* parcel_status_to_string(ParcelStatus status);

void parcel_print(const Parcel* parcel);

#endif /* PARCEL_H */
