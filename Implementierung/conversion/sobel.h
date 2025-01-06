#ifndef SOBEL_H
#define SOBEL_H

#include <stddef.h>
#include <stdint-gcc.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>

typedef struct {
    const uint8_t* img;
    size_t width;
    size_t height;
    float a, b, c;
    uint8_t* grayscale_image;
    uint8_t* result;
    size_t start_row;
    size_t end_row;
} SobelArgs;

void sobel_naive( const uint8_t* img, size_t width, size_t height,
            float a, float b, float c,
            void* tmp,
            uint8_t* result);
        
void sobel_optimized(const uint8_t* img, size_t width, size_t height,
            float a, float b, float c,
            void* tmp,
            uint8_t* result);

void sobel_SIMD(const uint8_t* img, size_t width, size_t height,
            float a, float b, float c,
            void* tmp,
            uint8_t* result);

void* sobel_worker(void* args);

void sobel_multithreaded(const uint8_t* img, size_t width, size_t height,
                         float a, float b, float c, void* tmp, uint8_t* result);
#endif