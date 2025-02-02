#ifndef PROJEKT_GRAYSCALE_TEST_H
#define PROJEKT_GRAYSCALE_TEST_H

#include "test.h"
#include "../conversion/grayscale.h"

void test_img_to_grayscale_naive (void);

void test_img_to_grayscale_naive_little_weights (void);

void test_img_to_grayscale_simd(void);

void test_img_to_grayscale_simd_5_pixels(void);

void test_img_to_grayscale_loop_unroll(void);

void test_img_to_grayscale_bitshift(void);

void test_img_to_grayscale_simd_8_pixels(void);

#endif
