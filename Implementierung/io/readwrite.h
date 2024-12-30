#ifndef READWRITE_H
#define READWRITE_H
#define _POSIX_C_SOURCE 200809L

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

typedef struct {
    char* mapped_file;
    size_t start;
    size_t size;
    uint8_t *buffer;
} ThreadData;

typedef struct {
    const uint8_t* data; // Pointer to the pixel data
    size_t start;        // Start offset in the data
    size_t size;         // Size of the chunk to write
    int fd;              // File descriptor
} ThreadData_write;

int open_and_validate_file(const char* filename, size_t* file_size);

size_t parse_ppm_header(const char* file_data, int* width, int* height, int* max_val);

uint8_t* allocate_pixel_memory(int width, int height);

void read_ppm_file(const char* filename, int* width, int* height, uint8_t** pixel_rgb_data);

void* read_chunk(void* arg);

void read_ppm_file_parallel_mmap(const char* filename, int* width, int* height, uint8_t** pixel_rgb_data);

void write_pgm_file(const char* filename, const uint8_t* data, int width, int height);

void* write_chunk(void* arg);

void write_pgm_file_parallel(const char* filename, const uint8_t* data, int width, int height);

#endif