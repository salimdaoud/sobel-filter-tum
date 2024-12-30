#include "readwrite.h"

// Helper function: Open and validate file
int open_and_validate_file(const char* filename, size_t* file_size) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        exit(1);
    }
    struct stat statbuf;
    if (fstat(fd, &statbuf) < 0) {
        perror("Error retrieving file stats");
        close(fd);
        exit(1);
    }

    if (!S_ISREG(statbuf.st_mode) || statbuf.st_size <= 0) {
        fprintf(stderr, "Error processing file: Not a regular file or invalid size\n");
        close(fd);
        exit(1);
    }

    *file_size = statbuf.st_size;
    return fd;
}

size_t parse_ppm_header(const char* file_data, int* width, int* height, int* max_val) {
    const char* current = file_data;

    // Parse the magic number (1st line)
    char magic_number[3];
    sscanf(current, "%2s", magic_number);
    if (strcmp(magic_number, "P6")) {
        fprintf(stderr, "Unsupported file format. P6 expected.\n");
        exit(1);
    }
    current = strchr(current, '\n') + 1; // Move past the first line

    // Parse the image dimensions (2nd line)
    sscanf(current, "%d %d", width, height);
    current = strchr(current, '\n') + 1; // Move past the second line

    // Parse the max value (3rd line)
    sscanf(current, "%d", max_val);
    current = strchr(current, '\n') + 1; // Move past the third line

    // Return the size of the header
    size_t header_size = current - file_data;
    return header_size;
}


// Helper function: Allocate memory
uint8_t* allocate_pixel_memory(int width, int height) {
    size_t pixel_count = width * height * 3;
    uint8_t* pixel_data = (uint8_t*)malloc(pixel_count);
    if (!pixel_data) {
        perror("Error allocating memory for pixel data");
        exit(1);
    }
    return pixel_data;
}


// Main function: Read PPM file sequentially
void read_ppm_file(const char* filename, int* width, int* height, uint8_t** pixel_rgb_data) {
    // Open and validate the file
    size_t file_size;
    int fd = open_and_validate_file(filename, &file_size);

    // Open file as FILE* for fread
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file for reading");
        close(fd);
        exit(1);
    }

    // Read the header into a buffer
    char header_data[1024]; // Assume header fits within 1 KB
    size_t header_read = fread(header_data, 1, sizeof(header_data), file);
    if (header_read == 0) {
        fprintf(stderr, "Error reading header data.\n");
        fclose(file);
        close(fd);
        exit(1);
    }

    // Parse the header
    int max_val;
    size_t header_size = parse_ppm_header(header_data, width, height, &max_val);

    // Reposition the file pointer after the header
    fseek(file, header_size, SEEK_SET);

    // Allocate memory for pixel data
    *pixel_rgb_data = allocate_pixel_memory(*width, *height);

    // Read pixel data
    if (fread(*pixel_rgb_data, 1, (*width) * (*height) * 3, file) != (size_t)((*width) * (*height) * 3)) {
        fprintf(stderr, "Error reading pixel data.\n");
        free(*pixel_rgb_data);
        fclose(file);
        close(fd);
        exit(1);
    }

    fclose(file);
    close(fd);
}

void* read_chunk(void* arg) {
    ThreadData *data = (ThreadData *)arg;

    // Pointer to the start of the chunk in the memory-mapped file
    const char *chunk_start = data->mapped_file + data->start;

    // Copy the chunk to the thread's buffer
    memcpy(data->buffer, chunk_start, data->size);

    return NULL;
}


// Main function: Read PPM file in parallel
void read_ppm_file_parallel_mmap(const char* filename, int* width, int* height, uint8_t** pixel_rgb_data) {
    // Open and validate the file
    size_t file_size;
    int fd = open_and_validate_file(filename, &file_size);

    // Memory-map the file
    char* mapped_file = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped_file == MAP_FAILED) {
        perror("Error mapping file");
        close(fd);
        exit(1);
    }

    // Parse the header
    int max_val;
    size_t header_size = parse_ppm_header(mapped_file, width, height, &max_val);

    // Allocate memory for pixel data
    *pixel_rgb_data = allocate_pixel_memory(*width, *height);

    // Divide work among threads
    int num_threads = 4; // Adjust based on system capabilities
    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];

    size_t pixel_count = (*width) * (*height) * 3;
    size_t chunk_size = (pixel_count + num_threads - 1) / num_threads;

    for (int i = 0; i < num_threads; i++) {
        thread_data[i].mapped_file = mapped_file;
        thread_data[i].start = header_size + i * chunk_size;
        thread_data[i].size = (i == num_threads - 1) ? (pixel_count - i * chunk_size) : chunk_size;
        thread_data[i].buffer = *pixel_rgb_data + i * chunk_size;

        pthread_create(&threads[i], NULL, read_chunk, &thread_data[i]);
    }

    // Wait for threads to complete
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    munmap(mapped_file, file_size);
    close(fd);
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

void* write_chunk(void* arg) {
    ThreadData_write *thread_data = (ThreadData_write *)arg;

    if (pwrite(thread_data->fd, thread_data->data + thread_data->start, thread_data->size, thread_data->start) < 0) {
        perror("Error writing chunk");
    }

    return NULL;
}

void write_pgm_file_parallel(const char* filename, const uint8_t* data, int width, int height){
   // Open the file
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("Error opening file");
        return;
    }

    // Write the header
    char header[128];
    int header_size = snprintf(header, sizeof(header), "P5\n%d %d\n255\n", width, height);
    if (write(fd, header, header_size) < 0) {
        perror("Error writing header");
        close(fd);
        return;
    }

    int num_threads = 4;
    // Divide the data into chunks
    size_t total_size = width * height;
    size_t chunk_size = (total_size + num_threads - 1) / num_threads;

    
    pthread_t threads[num_threads];
    ThreadData_write thread_data[num_threads];

    for (int i = 0; i < num_threads; i++) {
        thread_data[i].data = data;
        thread_data[i].start = header_size + i * chunk_size;
        thread_data[i].size = (i == num_threads - 1) ? (total_size - i * chunk_size) : chunk_size;
        thread_data[i].fd = fd;

        pthread_create(&threads[i], NULL, write_chunk, &thread_data[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    close(fd);
}