#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char* trim(char* str) {
    char* end;
    if (!str) return NULL;
    while (isspace((unsigned char)*str)) str++;
    if (*str == '\0') return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
    return str;
}

void config_set_defaults(Config* config) {
    const char* default_cities[] = {"Chennai", "Madurai", "Coimbatore", "Trichy", "Salem"};
    int i;
    if (!config) return;

    config->queue_capacity = 30;
    config->terminal_rotation_interval = 5;
    config->sort_per_tick = 3;
    config->reprocess_per_tick = 2;
    config->dispatch_per_tick = 2;
    config->misrouting_rate = 0.10;
    config->random_seed = 42;

    config->city_count = 5;
    for (i = 0; i < 5; i++) {
        strncpy(config->cities[i], default_cities[i], MAX_CITY_NAME_LEN - 1);
        config->cities[i][MAX_CITY_NAME_LEN - 1] = '\0';
    }
}

static void parse_city_list(Config* config, const char* str) {
    char buffer[1024];
    char* token;
    strncpy(buffer, str, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    config->city_count = 0;
    token = strtok(buffer, ",");
    while (token && config->city_count < MAX_CITIES) {
        char* clean_token = trim(token);
        if (strlen(clean_token) > 0) {
            /* Check for duplicate city names */
            int duplicate = 0;
            int j;
            for (j = 0; j < config->city_count; j++) {
                if (strcmp(config->cities[j], clean_token) == 0) {
                    duplicate = 1;
                    break;
                }
            }
            if (!duplicate) {
                strncpy(config->cities[config->city_count], clean_token, MAX_CITY_NAME_LEN - 1);
                config->cities[config->city_count][MAX_CITY_NAME_LEN - 1] = '\0';
                config->city_count++;
            }
        }
        token = strtok(NULL, ",");
    }
}

bool config_load(Config* config, const char* filename) {
    FILE* fp;
    char line[512];
    if (!config) return false;
    config_set_defaults(config);

    if (!filename) return false;

    fp = fopen(filename, "r");
    if (!fp) {
        printf("[Notice] Config file '%s' not found. Using standard default parameters.\n", filename);
        return false;
    }

    while (fgets(line, sizeof(line), fp)) {
        char* eq;
        char* key;
        char* val;
        char* trimmed = trim(line);
        if (trimmed[0] == '#' || trimmed[0] == '\0') {
            continue; /* Skip comments and blank lines */
        }
        eq = strchr(trimmed, '=');
        if (!eq) continue;

        *eq = '\0';
        key = trim(trimmed);
        val = trim(eq + 1);

        if (strcmp(key, "QUEUE_CAPACITY") == 0 || strcmp(key, "QUEUE CAPACITY") == 0) {
            config->queue_capacity = atoi(val);
        } else if (strcmp(key, "TERMINAL_ROTATION_INTERVAL") == 0 || strcmp(key, "TERMINAL ROTATION INTERVAL") == 0) {
            config->terminal_rotation_interval = atoi(val);
        } else if (strcmp(key, "SORT_PER_TICK") == 0 || strcmp(key, "SORT PER TICK") == 0) {
            config->sort_per_tick = atoi(val);
        } else if (strcmp(key, "REPROCESS_PER_TICK") == 0 || strcmp(key, "REPROCESS PER TICK") == 0) {
            config->reprocess_per_tick = atoi(val);
        } else if (strcmp(key, "DISPATCH_PER_TICK") == 0 || strcmp(key, "DISPATCH PER TICK") == 0) {
            config->dispatch_per_tick = atoi(val);
        } else if (strcmp(key, "MISROUTING_RATE") == 0 || strcmp(key, "MISROUTING RATE") == 0) {
            config->misrouting_rate = atof(val);
        } else if (strcmp(key, "RANDOM_SEED") == 0 || strcmp(key, "RANDOM SEED") == 0) {
            config->random_seed = (unsigned int)atoi(val);
        } else if (strcmp(key, "CITY_LIST") == 0 || strcmp(key, "CITY LIST") == 0) {
            parse_city_list(config, val);
        }
    }
    fclose(fp);

    /* Enforce safe positive bounds for all retained parameters */
    if (config->queue_capacity <= 0) config->queue_capacity = 30;
    if (config->terminal_rotation_interval <= 0) config->terminal_rotation_interval = 5;
    if (config->sort_per_tick <= 0) config->sort_per_tick = 3;
    if (config->reprocess_per_tick <= 0) config->reprocess_per_tick = 2;
    if (config->dispatch_per_tick <= 0) config->dispatch_per_tick = 2;
    if (config->misrouting_rate < 0.0) config->misrouting_rate = 0.0;
    if (config->misrouting_rate > 1.0) config->misrouting_rate = 1.0;
    if (config->city_count <= 0) {
        config_set_defaults(config);
    }
    return true;
}

void config_print(const Config* config) {
    int i;
    if (!config) return;
    printf("============================================================\n");
    printf("                SIMULATION CONFIGURATION                    \n");
    printf("============================================================\n");
    printf("  QUEUE_CAPACITY             : %d\n", config->queue_capacity);
    printf("  TERMINAL_ROTATION_INTERVAL : %d ticks\n", config->terminal_rotation_interval);
    printf("  SORT_PER_TICK              : %d\n", config->sort_per_tick);
    printf("  REPROCESS_PER_TICK         : %d\n", config->reprocess_per_tick);
    printf("  DISPATCH_PER_TICK          : %d\n", config->dispatch_per_tick);
    printf("  MISROUTING_RATE            : %.2f (%.1f%%)\n", config->misrouting_rate, config->misrouting_rate * 100.0);
    printf("  RANDOM_SEED                : %u%s\n", config->random_seed, config->random_seed == 0 ? " (time randomized)" : " (deterministic)");
    printf("  CITY_LIST (%d destinations): ", config->city_count);
    for (i = 0; i < config->city_count; i++) {
        printf("%s%s", config->cities[i], (i + 1 < config->city_count) ? ", " : "\n");
    }
    printf("============================================================\n");
}
