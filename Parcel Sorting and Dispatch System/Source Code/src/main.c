#include "simulation.h"
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
    const char* config_path = "config.txt";
    Simulation* sim;
    int i;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--config") == 0 && i + 1 < argc) {
            config_path = argv[++i];
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Usage: %s [options]\n", argv[0]);
            printf("Options:\n");
            printf("  --config <path>    Specify custom configuration file (default: config.txt)\n");
            printf("  --help             Show this help message\n");
            return 0;
        }
    }

    sim = sim_create(config_path);
    if (!sim) {
        fprintf(stderr, "Fatal: Could not initialize simulation.\n");
        return 1;
    }

    ui_run_interactive(sim);

    sim_free(sim);
    printf("All resources successfully deallocated. Thank you!\n");
    return 0;
}
