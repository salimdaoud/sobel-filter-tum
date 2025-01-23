#ifndef PROJEKT_TIME_MEASUREMENT_H
#define PROJEKT_TIME_MEASUREMENT_H

#define _POSIX_C_SOURCE 199309L

#include <bits/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

extern struct timespec* time_start;

extern struct timespec* time_end;

void start_time_measurement();

void end_time_measurement(char* function_name);

void clean_up_time_measurement();

#endif
