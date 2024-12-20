#include <getopt.h> //for struct option
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct ParsedArgs {
    int v_flag;          // Implementation version
    int b_flag;          // Flag for benchmarking or timing
    int repetitions;     // Number of repetitions
    char *input_file;    // Input file name
    char *output_file;   // Output file name
    double coeffs[3];     // Coefficients for conversion
};

void print_help();

int arg_parser (int argc, char *argv[], struct ParsedArgs *args);

