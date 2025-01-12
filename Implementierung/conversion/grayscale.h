#ifndef GRAYSCALE_H
#define GRAYSCALE_H

#include <stdint.h>
#include <stdlib.h>
#include <immintrin.h>
#include <stdbool.h>

#include "../util/time_measurement.h"

void img_to_grayscale(const uint8_t* img, size_t width, size_t height,
               float a, float b, float c, uint8_t* tmp);

void img_to_grayscale_SIMD(const uint8_t* img, size_t width, size_t height,
               float a, float b, float c, uint8_t* gray);

void img_to_grayscale_bitshift(const uint8_t* img, size_t width, size_t height,
                               float a, float b, float c, uint8_t* gray);

void img_to_grayscale_naive(const uint8_t* img, size_t width, size_t height,
                            float a, float b, float c, uint8_t* gray);

#endif