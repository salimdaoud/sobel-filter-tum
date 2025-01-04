#ifndef SOBEL_H
#define SOBEL_H

#include <stddef.h>
#include <stdint-gcc.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>

void sobel( const uint8_t* img, size_t width, size_t height,
            float a, float b, float c,
            void* tmp,
            uint8_t* result);

void sobel_optimization_v1(const uint8_t* img, size_t width, size_t height,
                           float a, float b, float c,
                           void* tmp,
                           uint8_t* result);

void sobel_optimization_v2(const uint8_t* img, size_t width, size_t height,
                           float a, float b, float c,
                           void* tmp,
                           uint8_t* result);

void sobel_optimization_v3(const uint8_t* img, size_t width, size_t height,
                           float a, float b, float c,
                           void* tmp,
                           uint8_t* result);

void sobel_optimization_v4(const uint8_t* img, size_t width, size_t height,
                           float a, float b, float c,
                           void* tmp,
                           uint8_t* result);




#endif