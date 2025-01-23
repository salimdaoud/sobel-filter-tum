#include <stdio.h>
#include <stdlib.h>

#include "io/readwrite.h"
#include "conversion/sobel.h"
#include "io/arg_parser.h"
#include "util/time_measurement.h"
#include "test/test.h"

int main(int argc, char* argv[]) {

    struct ParsedArgs args;

    // Parse command line arguments.
    if (arg_parser(argc, argv, &args) == -1) {
        return EXIT_FAILURE;
    }

    if (args.test_flag) {
        goto test_only;
    }

    int width, height;
    uint8_t* rgbData = NULL;

    // We need to pass a pointer to the pointer of rgbData to be able to change the pointer globally, not just the copy.
    read_ppm_file(args.input_file, &width, &height, &rgbData, true);

    // Allocate temporary buffer for grayscale and output buffer for the sobel filter result.
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

    switch (args.version_flag) {
        case 1:
            printf("Squareroot lookup Sobel implementation used.\n");
            if (args.benchmark_flag){
                start_time_measurement();
            }
            for (size_t i = 0; i < args.repetitions; i++) {
                sobel_squareroot_lookup_V1(rgbData, width, height, r_value_weighted, g_value_weighted,
                                        b_value_weighted, tmp, result);
            }
            if (args.benchmark_flag){
                end_time_measurement("Sobel Squareroot Lookup");
            }
            break;
        case 2:
            printf("Kernel unroll Sobel implementation used.\n");
            if (args.benchmark_flag){
                start_time_measurement();
            }
            for (size_t i = 0; i < args.repetitions; i++) {
                sobel_kernel_unroll_V2(rgbData, width, height, r_value_weighted, g_value_weighted,
                                    b_value_weighted, tmp, result);
            }
            if (args.benchmark_flag){
                end_time_measurement("Sobel Kernel Unroll");
            }
            break;
        case 3:
            if (width >= 4) {
                printf("SIMD Sobel implementation used.\n");
                if (args.benchmark_flag){
                    start_time_measurement();
                }
                for (size_t i = 0; i < args.repetitions; i++) {
                    sobel_SIMD_V3(rgbData, width, height, r_value_weighted, g_value_weighted,
                                b_value_weighted, tmp, result);
                }
                if (args.benchmark_flag){
                    end_time_measurement("Sobel SIMD implementation");
                }
            } else {
                printf("Image pixel width is too small. Using SIMD does not make sense.\n"
                        "Standard Sobel implementation used.\n");
                if (args.benchmark_flag){
                    start_time_measurement();
                }
                for (size_t i = 0; i < args.repetitions; i++) {
                    sobel_naive_V0(rgbData, width, height, r_value_weighted, g_value_weighted,
                                b_value_weighted, tmp, result);
                }
                if (args.benchmark_flag){
                    end_time_measurement("Naive Sobel Implementation");
                }
                break;
                }

            break;
        case 4:
            printf("Separated Convolution Sobel implementation used.\n");
            if (args.benchmark_flag){
                start_time_measurement();
            }
            for (size_t i = 0; i < args.repetitions; i++) {
                sobel_separated_convolution_V4(rgbData, width, height, r_value_weighted, g_value_weighted,
                                            b_value_weighted, tmp, result);
            }
            if (args.benchmark_flag){
                end_time_measurement("Sobel Separated Convolution");
            }
            break;
        default:
            printf("Standard Sobel implementation used.\n");
            if (args.benchmark_flag){
                start_time_measurement();
            }
            for (size_t i = 0; i < args.repetitions; i++) {
                sobel_naive_V0(rgbData, width, height, r_value_weighted, g_value_weighted,
                            b_value_weighted, tmp, result);
            }
            if (args.benchmark_flag){
                end_time_measurement("Naive Sobel Implementation");
            }
            break;
        }

    

    // Write result to PGM
    if (args.output_file != NULL){
        // TODO: decide how to treat read write threading
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
        // TODO: decide how to treat read write threading
        write_pgm_file(args.output_file, result, width, height, false);
    }

    // Free memory.
    clean_up_time_measurement();
    free(rgbData);
    free(tmp);
    free(result);


    test_only:
    if (args.test_flag) {
        #include "test/sobel_test.h"
        #include "test/readwrite_test.h"
        #include "test/grayscale_test.h"

        printf("\n");

        // Grayscale functions.
        test_img_to_grayscale_naive();
        test_img_to_grayscale_naive_little_weights();
        test_img_to_grayscale_SIMD();
        test_img_to_grayscale();
        test_img_to_grayscale_bitshift();
        test_img_to_grayscale_SIMD_8_pixels();

        // Sobel functions.
        test_sobel_naive_V0();
        test_sobel_kernel_unroll_V2();
        test_sobel_SIMD_V3();
        test_sobel_squareroot_lookup_V1();
        test_sobel_separated_convolution_V4();

        // Read/Write
        test_parse_ppm_header_correct_header();
        test_parse_ppm_header_incorrect_header();
        test_read_ppm_correct_file();
        test_read_ppm_correct_file_parallel();
        test_read_ppm_incorrect_file();
        // TODO: fix read write tests and adapt them to fit into test scheme
        // test_write_pgm_file();
        // test_read_ppm_incorrect_file_maxval();

        int test_result = (global_failed_tests != 0);

        printf("============================================================================================"
               "======================\n"
               "Test Run %s: %d of %d passed. %d failed.\n",
               test_result ? "FAILED" : "SUCCESSFUL",
               global_total_tests - global_failed_tests,
               global_total_tests, global_failed_tests);

        // This doesn't have any effect but suppressing a compiler warning.
        (void) function_name;
        (void) file;
    }
    
    
    return 0;


}