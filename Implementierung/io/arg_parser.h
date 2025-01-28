#include <getopt.h> //for struct option
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../test/test_runner.h"

struct ParsedArgs {
    size_t version_number;
    bool benchmark_flag;
    size_t repetitions;
    char *input_file;
    char *output_file;
    double rgb_coeffs[3];
};

void print_help();

int parse_arguments (int argc, char *argv[], struct ParsedArgs *args);

