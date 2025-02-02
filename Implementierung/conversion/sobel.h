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

void sobel(const uint8_t* img, size_t width, size_t height,
           float a, float b, float c,
           void* tmp,
           uint8_t* result);
        
void sobel_V1(const uint8_t* img, size_t width, size_t height,
              float a, float b, float c,
              void* tmp,
              uint8_t* result);

void sobel_V2(const uint8_t* img, size_t width, size_t height,
              float a, float b, float c,
              void* tmp,
              uint8_t* result);

void sobel_V3(const uint8_t* img, size_t width, size_t height,
              float a, float b, float c,
              void* tmp,
              uint8_t* result);

void sobel_V4(const uint8_t* img, size_t width, size_t height,
                             float a, float b, float c,
                             void* tmp,
                             uint8_t* result);
#endif