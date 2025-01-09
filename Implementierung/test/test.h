#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../conversion/grayscale.h"
#include "../conversion/sobel.h"

void assert_uint8_array_equal(const uint8_t* expected, const uint8_t* actual, size_t size);

void assert_int_equal(int expected, int actual);

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

void test_parse_ppm_header_correct_header(void);

void test_parse_ppm_header_incorrect_header(void);

void test_read_ppm_correct_file(void);

void test_read_ppm_correct_file_parallel(void);

void test_read_ppm_incorrcet_file(void);

void test_write_pgm_file(void);

