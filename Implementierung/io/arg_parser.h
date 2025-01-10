#include <getopt.h> //for struct option
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct ParsedArgs {
    size_t version_flag;
    _Bool benchmark_flag;
    _Bool test_flag;
    size_t repetitions;
    char *input_file;
    char *output_file;
    double rgb_coeffs[3];
};

void print_help();

int arg_parser (int argc, char *argv[], struct ParsedArgs *args);

