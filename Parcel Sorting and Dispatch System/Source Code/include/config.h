#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

#define MAX_CITIES 32
#define MAX_CITY_NAME_LEN 64

typedef struct Config {
    int queue_capacity;
    int terminal_rotation_interval;
    int sort_per_tick;
    int reprocess_per_tick;
    int dispatch_per_tick;
    double misrouting_rate;
    unsigned int random_seed;
    char cities[MAX_CITIES][MAX_CITY_NAME_LEN];
    int city_count;
} Config;

void config_set_defaults(Config* config);
bool config_load(Config* config, const char* filename);
void config_print(const Config* config);

#endif /* CONFIG_H */
