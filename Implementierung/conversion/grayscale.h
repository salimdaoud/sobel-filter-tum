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

void img_to_grayscale_SIMD_optimized(const uint8_t* img, size_t width, size_t height,
                                     float a, float b, float c, uint8_t* gray);

void img_to_grayscale_simd_8_pixels(const uint8_t* img, size_t width, size_t height,
                                    float a, float b, float c, uint8_t* gray);

static __inline void extract_r_g_b_sorted(const __m128i r_5_bgr_4_bgr_3_bgr_2_bgr_1_bgr_0,
                                          const __m128i bgr_7_bgr_6_bg_5,
                                          __m128i* r_76543210,
                                          __m128i* g_76543210,
                                          __m128i* b_76543210);
static __inline __m128i convert_rgb_to_gray_8_pixels(__m128i r_76543210,
                                                     __m128i g_76543210,
                                                     __m128i b_76543210,
                                                     float r_value_weighted,
                                                     float g_value_weighted,
                                                     float b_value_weighted);

#endif