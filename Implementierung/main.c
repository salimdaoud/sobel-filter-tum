#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>



#include "io/readwrite.h"
#include "conversion/sobel.h" 
#include "io/arg_parser.h"



int main(int argc, char* argv[]) {
    struct ParsedArgs args;

    // Parse arguments
    if (arg_parser(argc, argv, &args) == -1) {
        return EXIT_FAILURE;
    }

    int width, height;
    uint8_t* rgbData = NULL;
    float r_value_weighted = 0.299f;
    float g_value_weighted = 0.587f;
    float b_value_weighted = 0.114f;


    // We need to pass a pointer to the pointer of rgbData to be able to change the pointer globally, not just the copy
    read_ppm_file_parallel_mmap(args.input_file, &width, &height, &rgbData);
    
    // Allocate temporary buffer for grayscale and output buffer for edges
    uint8_t* tmp = malloc(width * height);
    uint8_t* result = malloc(width * height);

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

    // Apply Sobel operator
    if(args.coeffs[0] == 0.0 && args.coeffs[1] == 0.0 && args.coeffs[2] == 0.0){
        int iterations = 100;
        for( int i = 0; i < iterations ; ++ i){
            sobel_SIMD(rgbData, width, height, r_value_weighted, g_value_weighted, b_value_weighted, tmp, result, args.time_flag);
        }
    }
    else {
        sobel_naive(rgbData, width, height, args.coeffs[0], args.coeffs[1], args.coeffs[2], tmp, result, args.time_flag);
    }

    // Write result to PGM
    if (args.output_file != NULL){
        write_pgm_file(args.output_file, result, width, height);}
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
            write_pgm_file(args.output_file, result, width, height);
    }

    // Free memory
    free(rgbData);
    free(tmp);
    free(result);

    return 0;
}