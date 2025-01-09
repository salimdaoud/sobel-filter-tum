#include "square_root.h"

uint8_t squareroot_lookup (int value) {
    int middle = 0;
    root_interval cmp_interval = {0, 0, 0};
    int lower;
    int higher;

    if (value < 4356) {
        lower = 0;
        higher = 65;
    } else if (value < 36846) {
        lower = 96;
        higher = 191;
    } else {
        lower = 192;
        higher = 255;
    }

    while (lower <= higher) {
        middle = (lower + higher) / 2;
        cmp_interval = root_intervals[middle];
        if (value >= cmp_interval.lower_bound && value <= cmp_interval.upper_bound) {
            return cmp_interval.square_root;
        } else if (value < cmp_interval.lower_bound) {
            higher = middle - 1;
        } else {
            lower = middle + 1;
        }
    }
    return 0;
}