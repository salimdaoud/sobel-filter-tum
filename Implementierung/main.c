#include <stdio.h>
#include <stdlib.h>

#include "io/readwrite.h"
#include "conversion/sobel.h"
#include "io/arg_parser.h"
#include "util/time_measurement.h"
#include "test/test.h"
#include "test/grayscale_test.h"
#include "test/sobel_test.h"
#include "test/readwrite_test.h"





int main(int argc, char* argv[]) {

    struct ParsedArgs args;
    size_t repetitions = 1; //args.repetitions TODO: fix arg parser

    // Parse arguments
    if (arg_parser(argc, argv, &args) == -1) {
        return EXIT_FAILURE;
    }

    int width, height;
    uint8_t* rgbData = NULL;

    // We need to pass a pointer to the pointer of rgbData to be able to change the pointer globally, not just the copy
    read_ppm_file(args.input_file, &width, &height, &rgbData, true);

    // Allocate temporary buffer for grayscale and output buffer for edges
    uint8_t* tmp = malloc(width * height);
    uint8_t* result = malloc(width * height);

    float r_value_weighted = args.rgb_coeffs[0];
    float g_value_weighted = args.rgb_coeffs[1];
    float b_value_weighted = args.rgb_coeffs[2];

    if (!tmp || !result) {
        fprintf(stderr, "Memory allocation failed\n");
        free(rgbData);
        rgbData = NULL;
        free(tmp);
        tmp = NULL;
        free(result);
        result = NULL;
        exit(1);
    }

    for (size_t i = 0; i < args.repetitions; i++) {
        switch (args.version_flag) {
            case 1:
                printf("Squareroot lookup Sobel implementation used.\n");
                sobel_squareroot_lookup(rgbData, width, height, r_value_weighted, g_value_weighted,
                                        b_value_weighted, tmp, result, args.benchmark_flag);
                break;
            case 2:
                printf("Kernel unroll Sobel implementation used.\n");
                sobel_kernel_unroll(rgbData, width, height, r_value_weighted, g_value_weighted,
                                    b_value_weighted, tmp, result, args.benchmark_flag);
                break;
            case 3:
                if (width >= 4) {
                    printf("SIMD Sobel implementation used.\n");
                    sobel_SIMD(rgbData, width, height, r_value_weighted, g_value_weighted,
                               b_value_weighted, tmp, result, args.benchmark_flag);
                } else {
                    printf("Image pixel width is too small. Using SIMD does not make sense.\n"
                           "Standard Sobel implementation used.\n");
                    sobel_naive(rgbData, width, height, r_value_weighted, g_value_weighted,
                                b_value_weighted, tmp, result, args.benchmark_flag);
                    break;
                }

                break;
            case 4:
                printf("Separated Convolution Sobel implementation used.\n");
                sobel_separated_convolution(rgbData, width, height, r_value_weighted, g_value_weighted,
                                            b_value_weighted, tmp, result, args.benchmark_flag);
                break;
            default:
                printf("Standard Sobel implementation used.\n");
                sobel_naive(rgbData, width, height, r_value_weighted, g_value_weighted,
                            b_value_weighted, tmp, result, args.benchmark_flag);
                break;
        }

    }

    // Write result to PGM
    if (args.output_file != NULL){
        write_pgm_file(args.output_file, result, width, height, true);
    } else {
        args.output_file = malloc(strlen(args.input_file) + 1 );
        if (args.output_file == NULL) {
            fprintf(stderr, "Memory allocation failed.\n");
            exit(1);
        }
        strcpy(args.output_file, args.input_file);
        char *dot = strrchr(args.output_file, '.');
        if (dot != NULL) {
            strcpy(dot, ".pgm"); // Replace the extension
        }
            write_pgm_file(args.output_file, result, width, height, false);
    }

    // Free memory
    clean_up_time_measurement();
    free(rgbData);
    free(tmp);
    free(result);

    if (args.test_flag) {
        test_img_to_grayscale_naive();
        test_img_to_grayscale_naive_little_weights();
        test_img_to_grayscale_SIMD();
        test_img_to_grayscale();
        test_img_to_grayscale_bitshift();
        test_sobel_naive();
        test_sobel_kernel_unroll();
        test_sobel_SIMD();
        test_sobel_squareroot_lookup();
        test_sobel_separated_convolution();
        test_parse_ppm_header_correct_header();
        test_parse_ppm_header_incorrect_header();
        test_read_ppm_correct_file();
        test_read_ppm_correct_file_parallel();
        test_read_ppm_incorrcet_file();
        test_write_pgm_file();
        test_read_ppm_incorrcet_file_maxval();
    }
    
    
    return 0;


}