#include "io.h"
#include "arg_parser.h"
#include <string.h>

// Main function
int main(int argc, char* argv[]) {
    struct ParsedArgs args;

    // Parse arguments
    if (arg_parser(argc, argv, &args) == -1) {
        return EXIT_FAILURE;
    }

    /*if (argc < 3) {
        printf(stderr, "Usage: %s <input.ppm> -o <output.pgm>\n", argv[0]);
        return 1;
    }*/

    int width, height;
    uint8_t* rgbData = NULL;

    // Read a PPM file
    readPPM(args.input_file, &width, &height, &rgbData);

    // Allocate buffers
    uint8_t* tmp = malloc(width * height); // Temporary buffer for grayscale
    uint8_t* result = malloc(width * height); // Output buffer for edges

    if (!tmp || !result) {
        fprintf(stderr, "Memory allocation failed\n");
        free(rgbData);
        free(tmp);
        free(result);
        exit(1);
    }

    // Apply Sobel operator
    if(args.coeffs[0] == 0.0 && args.coeffs[1] == 0.0 && args.coeffs[2] == 0.0){
        sobel(rgbData, width, height, 0.299f, 0.587f, 0.114f, tmp, result);
    }
    else {
        sobel(rgbData, width, height, args.coeffs[0], args.coeffs[1], args.coeffs[2], tmp, result);
    }

    // Write result to PGM
    if (args.output_file != NULL){
        writePGM(args.output_file, result, width, height);}
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
        writePGM(args.output_file, result, width, height);
    }

    // Free memory
    free(rgbData);
    free(tmp);
    free(result);

    return 0;
}
