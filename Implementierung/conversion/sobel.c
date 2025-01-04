#include "sobel.h"
#include "grayscale.h"
#include "../util/lookup_tables.h"
#include "../io/readwrite.h"
#include "../util/square_root.h"

// Sobel operator implementation
void sobel( const uint8_t* img, size_t width, size_t height,
            float a, float b, float c,
            void* tmp,
            uint8_t* result) {

    printf("Standard Sobel implementation used.\n");

    // Temporary buffer for grayscale image
    uint8_t* grayscale_image = (uint8_t*)tmp;

    // Grayscale conversion
    grayscale(img, width, height, a, b, c, grayscale_image);

    // Sobel edge detection
    int kern_vertical[3][3] = {
            {1, 0, -1},
            {2, 0, -2},
            {1, 0, -1}
    };
    int kern_horizontal[3][3] = {
            { 1,  2,  1},
            { 0,  0,  0},
            {-1, -2, -1}
    };

    double time = 0;
    double start = curtime();

    for (size_t ppm_y = 0; ppm_y < height; ppm_y++) {
        for (size_t ppm_x = 0; ppm_x < width; ppm_x++) {
            int sum_vertical = 0, sum_horizontal = 0;

            // Apply Sobel kernel to pixel
            for (int kern_y = -1; kern_y <= 1; kern_y++) {
                // Only apply kernel values within ppm grid
                if ((ppm_y + kern_y >= 0) && (ppm_y + kern_y < height)) {
                    for (int kern_x = -1; kern_x <= 1; kern_x++) {
                        // Only apply kernel values within ppm grid
                        if ((ppm_x + kern_x >= 0) && (ppm_x + kern_x < width)){
                            uint8_t pixel = grayscale_image[(kern_y + ppm_y) * width + (ppm_x + kern_x)];
                            // + 1 to correct the indexing for the kernel matrix
                            sum_vertical += kern_vertical[kern_y + 1][kern_x + 1] * pixel;
                            sum_horizontal += kern_horizontal[kern_y + 1][kern_x + 1] * pixel;
                        }
                        else continue;
                    }
                }
                else continue;
            }

            // Compute gradient magnitude
            int magnitude = (int) sqrt(sum_vertical * sum_vertical + sum_horizontal * sum_horizontal);
            if (magnitude > 255) {
                magnitude = 255;
            }
            result[ppm_y * width + ppm_x] = (uint8_t)magnitude;
        }
    }

    double end = curtime();
    time = end - start;
    printf("time passed: %f\n", time);
}

void sobel_optimization_v1(const uint8_t* img, size_t width, size_t height,
                           float a, float b, float c,
                           void* tmp,
                           uint8_t* result) {

    printf("V1 Sobel implementation used.\n");

    // Temporary buffer for grayscale image
    uint8_t* grayscale_image = (uint8_t*)tmp;

    // Grayscale conversion
    grayscale(img, width, height, a, b, c, grayscale_image);

    int row_indices [height];

    double time = 0;
    double start = curtime();

    for (size_t i = 0; i < height; i++) {
        row_indices[i] = i * width;
    }

    int kernel_row_indices[3] = {0, 3, 6};


    for (size_t ppm_y = 0; ppm_y < height; ppm_y++) {
        for (size_t ppm_x = 0; ppm_x < width; ppm_x++) {
            int sum_vertical = 0, sum_horizontal = 0;

            // Apply Sobel kernel to pixel
            for (int kern_y = -1; kern_y <= 1; kern_y++) {
                // Only apply kernel values within ppm grid
                if ((ppm_y + kern_y >= 0) && (ppm_y + kern_y < height)) {
                    for (int kern_x = -1; kern_x <= 1; kern_x++) {
                        // Only apply kernel values within ppm grid
                        if ((ppm_x + kern_x >= 0 ) && (ppm_x + kern_x < width) && !(kern_x == 0 && kern_y == 0)){
                            uint8_t pixel = grayscale_image[row_indices[ppm_y + kern_y] + (ppm_x + kern_x)];
                            // + 1 to correct the indexing for the kernel matrix
                            sum_vertical += kern_vertical_lookup[kernel_row_indices[kern_y + 1] + kern_x + 1][pixel];
                            sum_horizontal += kern_horizontal_lookup[kernel_row_indices[kern_y + 1] + kern_x + 1][pixel];
                        }
                        else continue;
                    }
                }
                else continue;
            }

            // Compute gradient magnitude
            int magnitude = (int) sqrt(sum_vertical * sum_vertical + sum_horizontal * sum_horizontal);
            if (magnitude > 255) {
                magnitude = 255;
            }
            result[row_indices[ppm_y] + ppm_x] = (uint8_t)magnitude;
        }
    }

    double end = curtime();
    time = end - start;
    printf("time passed: %f\n", time);

}

void sobel_optimization_v2(const uint8_t* img, size_t width, size_t height,
                           float a, float b, float c,
                           void* tmp,
                           uint8_t* result) {

    printf("V2 Sobel implementation used.\n");

    // Temporary buffer for grayscale image
    uint8_t* grayscale_image = (uint8_t*)tmp;

    // Grayscale conversion
    grayscale(img, width, height, a, b, c, grayscale_image);
    size_t arr_ct = 0;

    int row_indices [height];

    double time = 0;
    double start = curtime();

    for (size_t i = 0; i < height; i++) {
        row_indices[i] = i * width;
    }

    // Sobel edge detection
    int kern_vertical[3][3] = {
            {1, 0, -1},
            {2, 0, -2},
            {1, 0, -1}
    };

    int kern_horizontal[3][3] = {
            { 1,  2,  1},
            { 0,  0,  0},
            {-1, -2, -1}
    };

    for (size_t ppm_y = 0; ppm_y < height; ppm_y++) {
        for (size_t ppm_x = 0; ppm_x < width; ppm_x++) {
            int sum_vertical = 0, sum_horizontal = 0;

            // Apply Sobel kernel to pixel
            for (int kern_y = -1; kern_y <= 1; kern_y++) {
                // Only apply kernel values within ppm grid
                if ((ppm_y + kern_y >= 0) && (ppm_y + kern_y < height)) {
                    for (int kern_x = -1; kern_x <= 1; kern_x++) {
                        // Only apply kernel values within ppm grid, check for kernel values to be non zero first
                        if (!(kern_x == 0 && kern_y == 0) && (ppm_x + kern_x >= 0 ) && (ppm_x + kern_x < width)){
                            uint8_t pixel = grayscale_image[row_indices[ppm_y + kern_y] + (ppm_x + kern_x)];
                            // + 1 to correct the indexing for the kernel matrix
                            sum_vertical += kern_vertical[kern_y + 1][kern_x + 1] * pixel;
                            sum_horizontal += kern_horizontal[kern_y + 1][kern_x + 1] * pixel;
                        }
                        else continue;
                    }
                }
                else continue;
            }

            // Compute gradient magnitude
            uint8_t magnitude;
            if ((sum_horizontal >= 255 || sum_horizontal <= -255 || sum_vertical >= 255 || sum_vertical <= -255) ||
                ((sum_horizontal >= 181 || sum_horizontal <= -181) && (sum_vertical >= 181 || sum_vertical <= -181))){
                magnitude = 255;
            } else {
                uint32_t sum_kernel = (sum_vertical * sum_vertical + sum_horizontal * sum_horizontal);
                if (sum_kernel < 65025) {
                    magnitude = squareroot_lookup(sum_kernel);
                    //printf("%d\n", magnitude);
                } else {
                    magnitude = 255;
                    //printf("%d\n", magnitude);
                }
            }
            result[arr_ct++] = magnitude;
        }
    }

    double end = curtime();
    time = end - start;
    printf("time passed: %f\n", time);
}

void sobel_optimization_v3(const uint8_t* img, size_t width, size_t height,
                           float a, float b, float c,
                           void* tmp,
                           uint8_t* result) {

    printf("V3 Sobel implementation used.\n");

    // Temporary buffer for grayscale image
    uint8_t* grayscale_image = (uint8_t*)tmp;

    // Grayscale conversion
    grayscale(img, width, height, a, b, c, grayscale_image);

    size_t size = width * height;

    int *temporary_sum = malloc((size)*sizeof(int));
    int *temporary_sum_2 = malloc((size)*sizeof(int));

    int row_indices [height];

    double time = 0;
    double start = curtime();

    for (size_t i = 0; i < height; i++) {
        row_indices[i] = i * width;
    }

    int row = 0;
    int sum = 0;

    for (size_t ppm_y = 0; ppm_y < height; ppm_y++) {
        for (size_t ppm_x = 0; ppm_x < width; ppm_x++) {
            sum = 0;
            row = row_indices[ppm_y];
            if (ppm_x > 0) {
                sum = grayscale_image[row + ppm_x - 1];
            }
            sum += grayscale_image[row + ppm_x] * 2;
            if (ppm_x + 1 < width) {
                sum += grayscale_image[row + ppm_x + 1];
            }
            temporary_sum [row + ppm_x] = sum;
        }
    }

    for (size_t ppm_y = 0; ppm_y < height; ppm_y++) {
        for (size_t ppm_x = 0; ppm_x < width; ppm_x++) {
            sum = 0;
            row = row_indices[ppm_y];
            if (ppm_y > 0) {
                sum = - temporary_sum[row - width + ppm_x];
            }
            if (ppm_y + 1 < height) {
                sum += temporary_sum[row + width + ppm_x];
            }
            temporary_sum_2 [row + ppm_x] = sum * sum;
        }
    }

    for (size_t ppm_y = 0; ppm_y < height; ppm_y++) {
        for (size_t ppm_x = 0; ppm_x < width; ppm_x++) {
            sum = 0;
            row = row_indices[ppm_y];
            if (ppm_y > 0) {
                sum = grayscale_image[row - width + ppm_x];
            }
            sum += grayscale_image[row + ppm_x] * 2;
            if (ppm_y + 1 < height) {
                sum += grayscale_image[row + width + ppm_x];
            }
            temporary_sum [row + ppm_x] = sum;
        }
    }

    for (size_t ppm_y = 0; ppm_y < height; ppm_y++) {
        for (size_t ppm_x = 0; ppm_x < width; ppm_x++) {
            sum = 0;
            row = row_indices[ppm_y];
            if (ppm_x > 0) {
                sum = - temporary_sum[row + ppm_x - 1];
            }
            if (ppm_x + 1 < width) {
                sum += temporary_sum[row + ppm_x + 1];
            }
            temporary_sum_2 [row + ppm_x] += sum * sum;
        }
    }

    for (size_t i = 0; i < size; i ++) {
        if (temporary_sum_2[i] < 65025) {
            result[i] = (uint8_t) sqrt(temporary_sum_2[i]);
        } else {
            result [i] = 255;
        }

    }

    double end = curtime();
    time = end - start;
    printf("time passed: %f\n", time);

    free(temporary_sum);
    free(temporary_sum_2);

}
