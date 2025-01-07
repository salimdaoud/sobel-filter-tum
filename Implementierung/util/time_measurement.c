#include "time_measurement.h"

struct timespec* time_start = NULL;
struct timespec* time_end = NULL;

void start_time_measurement() {
    if (!time_start) {
        time_start = malloc(sizeof (struct timespec));
    }
    clock_gettime(CLOCK_MONOTONIC, time_start);
}

void end_time_measurement(char* function_name) {
    if (!time_end) {
        time_end = malloc(sizeof (struct timespec));
    }
    clock_gettime(CLOCK_MONOTONIC, time_end);
    double measured_time = (time_end->tv_sec - time_start->tv_sec) + 1e-9 * (time_end->tv_nsec - time_start->tv_nsec);
    printf("Time elapsed for %s: %f seconds\n",function_name, measured_time);
}

void clean_up_time_measurement() {
    if(time_start) {
        free(time_start);
        time_start = NULL;
    }
    if(time_end) {
        free(time_end);
        time_end = NULL;
    }
}