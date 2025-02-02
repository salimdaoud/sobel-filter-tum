#include "test_runner.h"

void run_all_tests() {
    printf("\n============================================================================================"
           "======================\n");

    // Grayscale functions.
    test_img_to_grayscale_naive();
    test_img_to_grayscale_naive_little_weights();
    test_img_to_grayscale_simd_5_pixels();
    test_img_to_grayscale_simd_8_pixels();
    test_img_to_grayscale_loop_unroll();
    test_img_to_grayscale_bitshift();

    // Sobel functions.
    test_sobel_naive_V0();
    test_sobel_kernel_unroll_V1();
    test_sobel_separated_convolution_V2();
    test_sobel_simd_V3();
    test_sobel_squareroot_lookup_V4();

    // Read/Write
    test_parse_ppm_header_correct_header();
    test_parse_ppm_header_incorrect_header();
    test_read_ppm_correct_file();
    test_read_ppm_correct_file_parallel();
    test_write_pgm_file();

    int test_result = (global_failed_tests != 0);

    printf("\n\nTest Run %s: %d of %d passed. %d failed.\n",
           test_result ? "FAILED" : "SUCCESSFUL",
           global_total_tests - global_failed_tests,
           global_total_tests, global_failed_tests);

    // This doesn't have any effect but suppressing a compiler warning.
    (void) function_name;
    (void) file;
}
