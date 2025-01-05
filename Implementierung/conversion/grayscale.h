#ifndef GRAYSCALE_H
#define GRAYSCALE_H

#include <stdint.h>
#include <stdlib.h>
#include <immintrin.h>

void img_to_grayscale(const uint8_t* img, size_t width, size_t height,
               float a, float b, float c, uint8_t* tmp, int time_flag);

void img_to_grayscale_SIMD(const uint8_t* img, size_t width, size_t height,
               float a, float b, float c,
               uint8_t* gray, int time_flag);

#endif