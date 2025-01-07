#ifndef LOOKUP_TABLES_H
#define LOOKUP_TABLES_H

#include <stddef.h>
#include <stdint.h>


extern int kern_vertical_lookup[9][256];

extern int kern_horizontal_lookup[9][256];

typedef struct {
    int lower_bound;
    int upper_bound;
    uint8_t square_root;
} root_interval;

extern root_interval root_intervals[255];

#endif