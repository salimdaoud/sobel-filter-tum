#ifndef SOBEL_H
#define SOBEL_H

#include <stddef.h>
#include <stdint-gcc.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>

void sobel_naive( const uint8_t* img, size_t width, size_t height,
            float a, float b, float c,
            void* tmp,
            uint8_t* result);
        
void sobel_kernel_unroll(const uint8_t* img, size_t width, size_t height,
            float a, float b, float c,
            void* tmp,
            uint8_t* result);

void sobel_SIMD(const uint8_t* img, size_t width, size_t height,
            float a, float b, float c,
            void* tmp,
            uint8_t* result);

void sobel_squareroot_lookup(const uint8_t* img, size_t width, size_t height,
                             float a, float b, float c,
                             void* tmp,
                             uint8_t* result);

void sobel_separated_convolution(const uint8_t* img, size_t width, size_t height,
                                 float a, float b, float c,
                                 void* tmp,
                                 uint8_t* result);

#endif