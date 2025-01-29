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
    if (parse_arguments(argc, argv, &args) == -1) {
        return EXIT_FAILURE;
    }

    int width, height;
    uint8_t* rgbData = NULL;

    // We need to pass a pointer to the pointer of rgbData to be able to change the pointer globally, not just the copy.
    read_ppm_file(args.input_file, &width, &height, &rgbData, true);

    // Allocate temporary buffer for grayscale and output buffer for the sobel filter result. +8 to avoid undefined
    // behaviour when reading beyond limits with SIMD
    uint8_t* tmp = malloc(width * height + 8);
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
        exit(EXIT_FAILURE);
    }

    switch (args.version_number) {
        case 1:
            printf("Kernel unroll Sobel implementation used.\n");
            if (args.benchmark_flag){
                start_time_measurement();
            }
            for (size_t i = 0; i < args.repetitions; i++) {
                sobel_V1(rgbData, width, height, r_value_weighted, g_value_weighted,
                         b_value_weighted, tmp, result);
            }
            if (args.benchmark_flag){
                end_time_measurement("Sobel Kernel Unroll");
            }
            break;
        case 2:
            printf("Separated Convolution Sobel implementation used.\n");
            if (args.benchmark_flag){
                start_time_measurement();
            }
            for (size_t i = 0; i < args.repetitions; i++) {
                sobel_V2(rgbData, width, height, r_value_weighted, g_value_weighted,
                         b_value_weighted, tmp, result);
            }
            if (args.benchmark_flag){
                end_time_measurement("Sobel Separated Convolution");
            }
            break;
        case 3:
            if (width >= 4) {
                printf("SIMD Sobel implementation used.\n");
                if (args.benchmark_flag){
                    start_time_measurement();
                }
                for (size_t i = 0; i < args.repetitions; i++) {
                    sobel_V3(rgbData, width, height, r_value_weighted, g_value_weighted,
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
                    sobel(rgbData, width, height, r_value_weighted, g_value_weighted,
                          b_value_weighted, tmp, result);
                }
                if (args.benchmark_flag){
                    end_time_measurement("Naive Sobel Implementation");
                }
                break;
                }
            break;
        case 4:
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
        default:
            printf("Standard Sobel implementation used.\n");
            if (args.benchmark_flag){
                start_time_measurement();
            }
            for (size_t i = 0; i < args.repetitions; i++) {
                sobel(rgbData, width, height, r_value_weighted, g_value_weighted,
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
            exit(EXIT_FAILURE);
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
    
    return EXIT_SUCCESS;
}