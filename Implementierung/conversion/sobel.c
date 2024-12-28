#include "sobel.h"
#include "grayscale.h"
#include <time.h>

// Sobel operator implementation
void sobel( const uint8_t* img, size_t width, size_t height,
            float a, float b, float c,
            void* tmp,
            uint8_t* result) {

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

    // Start time measurement
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (size_t ppm_y = 0; ppm_y < height; ppm_y++) {
        for (size_t ppm_x = 0; ppm_x < width; ppm_x++) {
            int sum_vertical = 0, sum_horizontal = 0;

            // Apply Sobel kernel to pixel
            for (int kern_y = -1; kern_y <= 1; kern_y++) {
                // Only apply kernel values within ppm grid
                if ((ppm_y + kern_y) < height) {
                    for (int kern_x = -1; kern_x <= 1; kern_x++) {
                        // Only apply kernel values within ppm grid
                        if ((ppm_x + kern_x) < width){
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
            if (magnitude > 255) magnitude = 255;
            result[ppm_y * width + ppm_x] = (uint8_t)magnitude;
        }
    }

    // End time measurement
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time = (double) (end.tv_sec - start.tv_sec) + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
    printf("Time elapsed: %f seconds\n", time);
}