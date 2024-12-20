#include "readwrite.h"

// Read PPM file from filename into rgbData
void read_ppm_file(const char* file_name, int* width, int* height, uint8_t** pixel_rgb_data) {

    FILE* file;

    // ppm is assumed to be in binary format
    if (!(file = fopen(file_name, "rb"))) {
        perror("Error opening file");
        exit(1);
    }

    struct stat statbuf;

    if (fstat(fileno(file), &statbuf)) {
        fprintf(stderr, "Error retrieving file stats\n");
        goto cleanup;
    }

    if (!S_ISREG(statbuf.st_mode) || statbuf.st_size <= 0) {
        fprintf(stderr,
                "Error processing file: Not a regular file or invalid size\n");
        goto cleanup;
    }

    // Read magic number from ppm-file-header to check file format
    char magicNumber[3];
    // Maximum value of colour scale
    int max_val;
    // Magic number 2 first digits of ppm-file
    fscanf(file, "%2s", magicNumber);

    if (strcmp(magicNumber, "P6") != 0) {
        fprintf(stderr, "Unsupported format. PPM file in binary format (P6) expected.\n");
        goto cleanup;
    }

    if ((fscanf(file, "%d %d %d", width, height, &max_val)) <= 0) {
        fprintf(stderr, "Error trying to read PPM meta data.\n");
        goto cleanup;
    }

    // Consume the newline after max_val
    fgetc(file);
    size_t pixel_count = (*width) * (*height) * 3;

    if (!(*pixel_rgb_data = malloc(pixel_count))) {
        fprintf(stderr, "Error reading file: Could not allocate enough memory\n");
        goto cleanup;
    }

    printf( "Storage allocated.\n");

    if (fread(*pixel_rgb_data, 1, pixel_count, file) != pixel_count) {
        fprintf(stderr, "Error reading file.");
        free(*pixel_rgb_data);
        *pixel_rgb_data = NULL;
        goto cleanup;
    }

    cleanup:
    if (file) {
        fclose(file);
    }
}

// Write PGM file from data to filename
void write_pgm_file(const char* filename, const uint8_t* data, int width, int height) {
    FILE* file;

    if (!(file = fopen(filename, "wb"))) {
        perror("Error opening file");
        return;
    }

    // Write header into output file
    if ((fprintf(file, "P5\n%d %d\n255\n", width, height)) < 0) {
        fprintf(stderr, "Error writing output file header.\n");
        goto cleanup;
    }

    if (fwrite(data, 1, width * height, file) < (size_t)(width * height)) {
        fprintf(stderr, "Error writing pixel data into output file.\n");
        goto cleanup;
    }

    cleanup:
    if (file) {
        fclose(file);
    }
}