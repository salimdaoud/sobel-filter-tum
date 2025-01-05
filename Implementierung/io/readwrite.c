#include "readwrite.h"

// Main function: Read PPM file sequentially.
void read_ppm_file(const char* file_name, int* width, int* height, uint8_t** pixel_rgb_data, bool use_io_threading) {
    // Open and validate the file
    size_t file_size;

    int file_descriptor = open_and_validate_file(file_name, &file_size);

    // Memory-map the file. Not necessarry for sequential implementation, but we wanted to make it uniform.
    char* mapped_file = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, file_descriptor, 0);
    if (mapped_file == MAP_FAILED) {
        perror("Error mapping file.\n");
        goto cleanup;
    }

    // Parse the header
    int max_val;
    size_t header_size = parse_ppm_header(mapped_file, width, height, &max_val);

    size_t image_size = *width * *height;
    size_t rgb_values = image_size * 3;

    if (!(*pixel_rgb_data = malloc(rgb_values))) {
        fprintf(stderr, "Error reading file: Could not allocate enough memory\n");
        goto cleanup;
    }

    if (use_io_threading) {
        int num_threads = 4;
        pthread_t threads[num_threads];
        ThreadData thread_data[num_threads];

        size_t thread_section_size = (rgb_values + num_threads - 1) / num_threads;

        for (int i = 0; i < num_threads; i++) {
            thread_data[i].mapped_file = mapped_file;
            thread_data[i].start = header_size + i * thread_section_size;
            thread_data[i].size = (i == num_threads - 1) ? (rgb_values - i * thread_section_size) : thread_section_size;
            thread_data[i].buffer = *pixel_rgb_data + i * thread_section_size;

            pthread_create(&threads[i], NULL, read_thread_section, &thread_data[i]);
        }

        // Wait for threads to complete
        for (int i = 0; i < num_threads; i++) {
            pthread_join(threads[i], NULL);
        }
    } else {
        memcpy(*pixel_rgb_data, (const char*) (mapped_file + header_size), rgb_values);
    }

    cleanup:
    if (mapped_file) {
        munmap(mapped_file, file_size);
    }
    if (file_descriptor) {
        close(file_descriptor);
    }
}

// Helper function: Open and validate file
int open_and_validate_file(const char* file_name, size_t* file_size) {
    int file_descriptor = open(file_name, O_RDONLY);

    if (file_descriptor == -1) {
        perror("Error opening file\n");
        exit(1);
    }

    struct stat statbuf;

    if (fstat(file_descriptor, &statbuf)) {
        fprintf(stderr, "Error retrieving file stats\n");
        return -1;
    }

    if (!S_ISREG(statbuf.st_mode) || statbuf.st_size <= 0) {
        fprintf(stderr,
                "Error processing file: Not a regular file or invalid size\n");
        return -1;
    }

    *file_size = statbuf.st_size;
    return file_descriptor;
}

size_t parse_ppm_header(char* file_data, int* width, int* height, int* max_val) {
    char* file_start = file_data;

    // Read magic number from ppm-file-header to check file format
    char magicNumber[3];
    // Magic number 2 first digits of ppm-file
    sscanf(file_data, "%2s", magicNumber);

    if (strcmp(magicNumber, "P6") != 0) {
        fprintf(stderr, "Unsupported format. PPM file in binary format (P6) expected.\n");
        return -1;
    }
    file_data = strchr(file_data, '\n') + 1;

    if ((sscanf(file_data, "%d %d", width, height) <= 0)) {
        fprintf(stderr, "Error trying to read image dimensions from PPM header.\n");
        return -1;
    }
    file_data = strchr(file_data, '\n') + 1;

    if ((sscanf(file_data, "%d", &max_val)) <= 0) {
        fprintf(stderr, "Error trying to read image max colour value from PPM header.\n");
        return -1;
    }
    file_data = strchr(file_data, '\n') + 1;

    // Skip new line after max value.
    file_data = strchr(file_data, '\n') + 1;

    // Return the size of the header
    size_t header_size = file_data - file_start;
    return header_size;
}

void* read_thread_section(void* arg) {
    ThreadData *data = (ThreadData *)arg;

    // Pointer to the start of the chunk in the memory-mapped file
    const char *chunk_start = data->mapped_file + data->start;

    // Copy the chunk to the thread's buffer
    memcpy(data->buffer, chunk_start, data->size);

    return NULL;
}

// Write PGM file from data to filename
void write_pgm_file(const char* filename, const uint8_t* data, int width, int height, bool use_io_threading) {
    size_t image_size = width * height;
    // Open the file
    int file_descriptor = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_descriptor < 0) {
        perror("Error opening file.\n");
        return;
    }

    // Write the header
    char header[128];
    int header_size = snprintf(header, sizeof(header), "P5\n%d %d\n255\n", width, height);
    if (write(file_descriptor, header, header_size) < 0) {
        perror("Error writing header.\n");
        goto cleanup;
    }

    if (use_io_threading) {
        int num_threads = 4;
        // Divide the data into chunks
        size_t chunk_size = (image_size + num_threads - 1) / num_threads;

        pthread_t threads[num_threads];
        ThreadData_write thread_data[num_threads];

        for (int i = 0; i < num_threads; i++) {
            thread_data[i].data = data;
            thread_data[i].start = header_size + i * chunk_size;
            thread_data[i].size = (i == num_threads - 1) ? (image_size - i * chunk_size) : chunk_size;
            thread_data[i].fd = file_descriptor;

            pthread_create(&threads[i], NULL, write_chunk, &thread_data[i]);
        }

        for (int i = 0; i < num_threads; i++) {
            pthread_join(threads[i], NULL);
        }
    } else {
        if (pwrite(file_descriptor, data, image_size, header_size) < 0) {
            fprintf(stderr, "Error writing pixel data into output file.\n");
            goto cleanup;
        }
    }

    cleanup:
    if (file_descriptor) {
        close(file_descriptor);
    }
}

void* write_chunk(void* arg) {
    ThreadData_write *thread_data = (ThreadData_write *)arg;

    if (pwrite(thread_data->fd, thread_data->data + thread_data->start, thread_data->size, thread_data->start) < 0) {
        fprintf(stderr, "Error writing pixel data into output file.\n");
        exit(1);
    }

    return NULL;
}