#ifndef READWRITE_H
#define READWRITE_H
#define _POSIX_C_SOURCE 199309L

#include <stddef.h>
#include <stdint-gcc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <bits/time.h>
#include <time.h>

void read_ppm_file(const char* fileName, int* width, int* height, uint8_t** rgbData);

void write_pgm_file(const char* filename, const uint8_t* data, int width, int height);

double curtime(void);

#endif