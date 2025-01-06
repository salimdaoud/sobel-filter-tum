#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>



#include "io/readwrite.h"
#include "conversion/sobel.h"
#include "io/arg_parser.h"
#include "util/time_measurement.h"



int main(int argc, char* argv[]) {
    struct ParsedArgs args;
    size_t repetitions = 1;

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

    float r_value_weighted = args.coeffs[0];
    float g_value_weighted = args.coeffs[1];
    float b_value_weighted = args.coeffs[2];

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

    //if (args.b_flag) {
    //    repetitions = args.repetitions;
    //    struct timespec start;
    //    clock_gettime (CLOCK_MONOTONIC , &start);
    //}

    for (size_t i = 0; i < repetitions; i++) {
        // Apply Sobel operator
        choose_implementation:
        switch (args.v_flag) {
            case 1:
                sobel_squareroot_lookup(rgbData, width, height, r_value_weighted, g_value_weighted,
                                        b_value_weighted, tmp, result);
                break;
            case 2:
                sobel_kernel_unroll(rgbData, width, height, r_value_weighted, g_value_weighted,
                                    b_value_weighted, tmp, result);
                break;
            case 3:
                sobel_SIMD(rgbData, width, height, r_value_weighted, g_value_weighted,
                           b_value_weighted, tmp, result);
                break;
            case 4:
                sobel_separated_convolution(rgbData, width, height, r_value_weighted, g_value_weighted,
                                            b_value_weighted, tmp, result);
                break;
            default:
                sobel_naive(rgbData, width, height, r_value_weighted, g_value_weighted,
                            b_value_weighted, tmp, result);
                break;
        }

    }

    //if (args.b_flag) {
    //    struct timespec end;
    //    clock_gettime (CLOCK_MONOTONIC , & end ) ;
    //    double time = end . tv_sec - start . tv_sec + 1e-9 * ( end . tv_nsec - start . tv_nsec );
    //    double avg_time = time / repetitions ;
    //    printf("Time elapsed: %f seconds\n", avg_time);
    //}

    // Write result to PGM
    if (args.output_file != NULL){
        write_pgm_file(args.output_file, result, width, height, false);}
    else {
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
            write_pgm_file(args.output_file, result, width, height, true);
    }

    // Free memory
    free(rgbData);
    free(tmp);
    free(result);

    return 0;
}