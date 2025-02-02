#include "arg_parser.h"

void print_help() {
    printf("\nThe program applies a Sobel based Edge Detection Filter on a PPM P6 file.\n"
           "Use it like this:\n\n"
           "%-7s program <inputfile> [options]\n", "");
    printf("\nOptions:\n");
    printf("%-7s [%s] <%s> %s\n", "", "-V", "Zahl", " -  Specify the implementation version (default: 0)");
    printf("%-7s [%s]=<%s> %s\n", "", "-B", "Zahl", " -  Measure the implementation version runtime with "
                                                    "optional repetitions (default: 1). No space between option"
                                                    " identifier and argument allowed.");
    printf("%-7s [%s] <%s> %-s\n", "", "-o", "Dateiname", " -  Specify output file");
    printf("%-7s [%s] <%s> <%s> <%s> %s\n", "", "--rgb_coeffs", "a", "b", "c", " -  Set coefficients a, b, and c "
                                                                               "for Grayscale conversion");
    printf("%-7s [%s] %s\n", "", "-t", " -  Run the tests");
    printf("%-7s [%s] %s\n", "", "-h/--help", " -  Show this help message and exit");
}

static bool check_file_extension(const char* filename, const char* extension) {
    const char* dot = strrchr(filename, '.');
    if(!dot || dot == filename) {
        return false;
    } else {
        return strcmp(dot + 1, extension) == 0;
    }
}

int parse_arguments (int argc, char *argv[], struct ParsedArgs *args) {
    int opt;
    int option_index = 0;

    // Default values
    args->version_number = 0;
    args->benchmark_flag = false;
    args->repetitions = 1;
    args->input_file = NULL;
    args->output_file = NULL;
    args->rgb_coeffs[0] = 0.299f;
    args->rgb_coeffs[1] = 0.587f;
    args->rgb_coeffs[2] = 0.114f;

    static struct option long_options[] = {
    {"help",    no_argument,       0, 'h'},
    {"rgb_coeffs",  required_argument, 0,  'c' },
    {0, 0, 0, 0}  // Terminating entry
    };


    while ((opt = getopt_long(argc, argv, "V:v:O:o:B::b::HhTt", long_options, &option_index)) != -1){
        switch (opt){
            case 'h': case 'H': // Help. Other flags will be ignored.
                print_help();
                exit(EXIT_SUCCESS);
                case 't': case 'T': // Tests. Other flags will be ignored.
                run_all_tests();
                exit(EXIT_SUCCESS);
            case 'v': case 'V': // Sobel Version
                if (optarg != NULL){
                    errno = 0;
                    char* endptr;
                    args->version_number = (size_t) strtol(optarg, &endptr, 10);
                    if (endptr == optarg || *endptr != '\0') {
                        fprintf(stderr, "Version argument '%s' could not be converted to long.\n", optarg);
                        exit(EXIT_FAILURE);
                    } else if (errno == ERANGE) {
                        fprintf(stderr, "Version argument '%s' over- or underflows long.\n", optarg);
                        exit(EXIT_FAILURE);
                    }
                    if (args->version_number > 4) {
                        fprintf(stderr, "Invalid version number. Please choose one of the following versions:\n\n"
                                                "%-7s [%d] %s\n"
                                                "%-7s [%d] %s\n"
                                                "%-7s [%d] %s\n"
                                                "%-7s [%d] %s\n"
                                                "%-7s [%d] %s\n",
                                                "", 0, "Naive Sobel Implementation",
                                                "", 1, "Unrolled Kernel",
                                                "", 2, "Separate Convolution",
                                                "", 3, "SIMD",
                                                "", 4, "Squareroot Lookup Table");
                        exit(EXIT_FAILURE);
                    }
                } else {
                    args->version_number = 0;
                }
                break;
            case 'b': case 'B': // Benchmarking
                args->benchmark_flag = true;
                if (optarg != NULL){
                    errno = 0;
                    char* endptr;
                    args->repetitions = (size_t) strtol(optarg, &endptr, 10);
                    if (endptr == optarg || *endptr != '\0') {
                        fprintf(stderr, "Benchmarking argument '%s' could not be converted to long.\n", optarg);
                        exit(EXIT_FAILURE);
                    } else if (errno == ERANGE) {
                        fprintf(stderr, "Benchmarking argument '%s' over- or underflows long.\n", optarg);
                    }
                }
                break;
            case 'o': case 'O': // Output file
                if (optarg != NULL){
                    if (check_file_extension(optarg, "pgm")) {
                        args->output_file = optarg;
                    } else {
                        fprintf(stderr, "Error: Incorrect output file format. Format must be of type '.pgm'.");
                        exit(EXIT_FAILURE);
                    }
                }
                break;
            case 'c':
                for (int8_t i = 0; i < 3; i++) {
                    if (optind + 1 >= argc) {
                        fprintf(stderr, "--rgb_coeffs requires 3 numeric values.\n");
                        exit(EXIT_FAILURE);
                    }
                    errno = 0;
                    char* endptr;
                    args->rgb_coeffs[i] = strtod(argv[optind + i - 1], &endptr);
                    if (endptr == argv[optind + i - 1] || *endptr != '\0') {
                        fprintf(stderr, "RGB coefficient argument '%s' (index [%d]) could not be converted "
                                        "to double.\n",argv[optind + i - 1], i);
                        exit(EXIT_FAILURE);
                    } else if (errno == ERANGE) {
                        fprintf(stderr, "RGB coefficient argument '%s' (index [%d]) over- or underflows"
                                        " double.\n", argv[optind + i - 1], i);
                        exit(EXIT_FAILURE);
                    }
                }
                if(fabs((args->rgb_coeffs[0] + args->rgb_coeffs[1] + args->rgb_coeffs[2]) - 1.0) > 0.0001) {
                    fprintf(stderr, "Incorrect RGB coefficients. RGB coefficients must add up to ~ 1.0.\n");
                    exit(EXIT_FAILURE);
                }
                optind += 2;
                break;
            default:
                print_help();
                exit(EXIT_FAILURE);
        }
    }

    if(optind < argc - 1) {
        fprintf(stderr, "Error: Incorrect CLI command format. Correct format specified in help section below:\n");
        print_help();
        exit(EXIT_FAILURE);
    }

    // Positional argument (input file).
    if (optind < argc) {
        if (check_file_extension(argv[optind], "ppm")) {
            args->input_file = argv[optind];
        } else {
            fprintf(stderr, "Error: Incorrect output file format. Format must be of type '.ppm'.");
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "Error: Input file is required.\n");
        print_help();
        exit(EXIT_FAILURE);
    }

    // Display parsed values.
    printf("\nInput file: %s\n", args->input_file);
    if (args->output_file) {
        printf("Output file: %s\n", args->output_file);
    }
    printf("\nGrayscaling coefficients: R = %.3f, G = %.3f, B = %.3f\n",
           args->rgb_coeffs[0], args->rgb_coeffs[1], args->rgb_coeffs[2]);
    printf("\nImplementation version: %ld\n", args->version_number);
    if (args->benchmark_flag) {
        printf("Runtime measurement enabled with %ld %s\n\n", args->repetitions, (args->repetitions > 1) ? "repetitions" : "repetition");
    }

    return EXIT_SUCCESS;
}