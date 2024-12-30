#define _POSIX_C_SOURCE 199309L
#include "sobel.h"
#include "grayscale.h"
#include <time.h>
#include <immintrin.h>
#include <stddef.h>
#include <stdint.h>

// Sobel operator implementation
void sobel( const uint8_t* img, size_t width, size_t height,
            float a, float b, float c,
            void* tmp,
            uint8_t* result) {

    // Temporary buffer for grayscale image
    uint8_t* grayscale_image = (uint8_t*)tmp;

    /*struct timespec start;
    int iterations = 5000;*/
    // Grayscale conversion
    //clock_gettime (CLOCK_MONOTONIC , &start);
    //for( int i = 0; i < iterations ; ++ i){
    img_to_grayscale(img, width, height, a, b, c, grayscale_image);
    //}
    /*struct timespec end;
    clock_gettime (CLOCK_MONOTONIC , & end ) ;
    double time = end . tv_sec - start . tv_sec + 1e-9 *
    ( end . tv_nsec - start . tv_nsec );
    double avg_time = time / iterations ;
    printf("%f\n", avg_time);*/

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
                if ((ppm_y + kern_y) >= 0 && (ppm_y + kern_y) < height) {
                    for (int kern_x = -1; kern_x <= 1; kern_x++) {
                        // Only apply kernel values within ppm grid
                        if ((ppm_x + kern_x) >= 0 && (ppm_x + kern_x) < width){
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
}