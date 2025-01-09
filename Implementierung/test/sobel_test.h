#ifndef PROJEKT_SOBEL_TEST_H
#define PROJEKT_SOBEL_TEST_H

#include "test.h"
#include "../conversion/sobel.h"

void test_sobel_naive (void);

void test_sobel_kernel_unroll(void);

void test_sobel_SIMD(void);

void test_sobel_squareroot_lookup (void);

void test_sobel_separated_convolution (void);


#endif
