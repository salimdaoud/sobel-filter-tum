#include <stdio.h>
#include "../conversion/grayscale.h"
#include "../conversion/sobel.h"

void assert_uint8_array_equal(const uint8_t* expected, const uint8_t* actual, size_t size);

void test_img_to_grayscale_naive (void);

void test_img_to_grayscale_naive_little_weights (void);

void test_img_to_grayscale_SIMD(void);

void test_img_to_grayscale(void);

void test_img_to_grayscale_bitshift(void);

void test_sobel_naive (void);

void test_sobel_kernel_unroll(void);

void test_sobel_SIMD(void);

void test_sobel_squareroot_lookup (void);

void test_sobel_separated_convolution (void);