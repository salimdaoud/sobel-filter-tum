#ifndef READWRITE_H
#define READWRITE_H
#define _POSIX_C_SOURCE 200809L

#include <unistd.h>


#include <stddef.h>
#include <stdint-gcc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <bits/time.h>
#include <time.h>

typedef struct {
    char* mapped_file;
    size_t start;
    size_t size;
    uint8_t *buffer;
} ThreadData;

typedef struct {
    const uint8_t* data;
    size_t start;
    size_t size;
    int file_descriptor;
    uint8_t header_offset;
} ThreadData_write;

int open_and_validate_file(const char* file_name, size_t* file_size);


size_t parse_ppm_header(char* file_data, int* width, int* height, int* max_val);

void read_ppm_file(const char* file_name, int* width, int* height, uint8_t** pixel_rgb_data, bool use_io_threading);

void* read_thread_section(void* arg);

void write_pgm_file(const char* filename, const uint8_t* sobel_data, int width, int height, bool use_io_threading);

void* write_thread_section(void* arg);

#endif