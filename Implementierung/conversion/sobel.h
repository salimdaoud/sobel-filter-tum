#ifndef SOBEL_H
#define SOBEL_H

#include "grayscale.h"
#include "../io/readwrite.h"
#include "../util/square_root.h"
#include "../util/time_measurement.h"

#include <stddef.h>
#include <stdint-gcc.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <emmintrin.h>
#include <stdint.h>


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

void sobel_naive_V0( const uint8_t* img, size_t width, size_t height,
            float a, float b, float c,
            void* tmp,
            uint8_t* result);
        
void sobel_kernel_unroll_V2(const uint8_t* img, size_t width, size_t height,
            float a, float b, float c,
            void* tmp,
            uint8_t* result);

void sobel_SIMD_V3(const uint8_t* img, size_t width, size_t height,
            float a, float b, float c,
            void* tmp,
            uint8_t* result);

void sobel_squareroot_lookup_V1(const uint8_t* img, size_t width, size_t height,
                             float a, float b, float c,
                             void* tmp,
                             uint8_t* result);

void sobel_separated_convolution_V4(const uint8_t* img, size_t width, size_t height,
                                 float a, float b, float c,
                                 void* tmp,
                                 uint8_t* result);

void* sobel_worker(void* args);

void sobel_multithreaded(const uint8_t* img, size_t width, size_t height,
                         float a, float b, float c, void* tmp, uint8_t* result);
#endif