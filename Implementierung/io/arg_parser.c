#include "arg_parser.h"

void print_help() {
    printf("Usage: program [options] <inputfile>\n");
    printf("Options:\n");
    printf("  -V <Zahl>            Specify the implementation version (default: 0)\n");
    printf("  -B [Zahl]            Measure runtime with optional repetitions\n");
    printf("  -o <Dateiname>       Specify output file\n");
    printf("  --rgb_coeffs <a> <b> <c> Set coefficients a, b, and c for conversion\n");
    printf("  -h, --help           Show this help message and exit\n");
    printf("  -t                   enable all tests\n");
}

int arg_parser (int argc, char *argv[], struct ParsedArgs *args) {
    int opt;
    int option_index = 0;

    // Default values
    args->version_flag = 0;
    args->benchmark_flag = 0;
    args->test_flag = 0;
    args->repetitions = 1;
    args->input_file = NULL;
    args->output_file = NULL;
    args->rgb_coeffs[0] = 0.299f;
    args->rgb_coeffs[1] = 0.587f;
    args->rgb_coeffs[2] = 0.114f;

    static struct option long_options[] = {
    {"help",    no_argument,       0, 'h'},
    {"rgb_coeffs",  required_argument, 0,  0 },
    {0, 0, 0, 0}  // Terminating entry
    };
  
    
    while ((opt = getopt_long(argc, argv, "V:B:T::v:b:o:h:t", long_options, &option_index)) != -1){
        switch (opt){
            case 'v': case 'V':
                if (optarg != NULL){
                    char* endptr;
                    args->version_flag = (size_t) strtol(optarg, &endptr, 10);
                }
                else {args->version_flag = 0;}
                break;
            case 'b': case 'B':
                args->benchmark_flag = 1;
                if (optarg != NULL){
                    char* endptr;
                    args->repetitions = (size_t) strtol(optarg, &endptr, 10);
                }
                break;
            case 'o': // Output file
                if (optarg != NULL){
                args->output_file = optarg;
                }
                // is an error required if there is no optarg?
                break;
            case 'h':
                print_help();
                exit(0);
            case 't' : case 'T':
                args->test_flag = 1;
                break; 
            case 0: // Long options like --rgb_coeffs

                //getopt_long updates option_index automatically when a long option is successfully matched.
                if (strcmp("rgb_coeffs", long_options[option_index].name) == 0) {
                    if (optind + 2 > argc) { // optind is used with functions like getopt and getopt_long to keep track of the next index in the argv array to be processed
                        fprintf(stderr, "Error: --rgb_coeffs requires three floating point arguments.\n");
                        exit(1); //review because not necessarily optind + 2 < argc
                                        // and there are three floating point arguments
                    }
                    char* endptr;
                    args->rgb_coeffs[0] = strtod(argv[optind - 1], &endptr);
                    args->rgb_coeffs[1] = strtod(argv[optind], &endptr);
                    args->rgb_coeffs[2] = strtod(argv[optind + 1], &endptr);
                    optind += 2; // Advance past extra arguments
                }
                break;
            default:
                print_help();
                exit(1);
        }
    }
    // Positional argument (input file)
    if (optind < argc) {
        args->input_file = argv[optind]; // optind points to positional argument, which is an input file
    } else {
        fprintf(stderr, "Error: Input file is required.\n");
        print_help();
        exit(1);
    }
    

    // Display parsed values
    printf("Implementation version: %ld\n", args->version_flag);
    if (args->benchmark_flag) {
        printf("Runtime measurement enabled with %ld repetitions\n", args->repetitions);
    }
    printf("Input file: %s\n", args->input_file);
    if (args->output_file) {
        printf("Output file: %s\n", args->output_file);
    }
    printf("Coefficients: a = %.3f, b = %.3f, c = %.3f\n", args->rgb_coeffs[0], args->rgb_coeffs[1], args->rgb_coeffs[2]);

    return 0;
}