#ifndef PROJEKT_SOBEL_TEST_H
#define PROJEKT_SOBEL_TEST_H

#include "test.h"
#include "../conversion/sobel.h"

void test_sobel_naive_V0(void);

void test_sobel_kernel_unroll_V1(void);

void test_sobel_separated_convolution_V2(void);

void test_sobel_simd_V3(void);

void test_sobel_squareroot_lookup_V4(void);


#endif
