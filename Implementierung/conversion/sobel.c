#define _POSIX_C_SOURCE 199309L
#include "sobel.h"
#include "grayscale.h"
#include <time.h>
#include <emmintrin.h>
#include <stddef.h>
#include <stdint.h>
#include "../io/readwrite.h"
#include "../util/square_root.h"
#include "../util/time_measurement.h"

// Sobel operator implementation
void sobel_naive( const uint8_t* img, size_t width, size_t height,
            float a, float b, float c,
            void* tmp,
            uint8_t* result) {

    printf("Standard Sobel implementation used.\n");

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

    // Start time measurement
    /*struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);*/

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

    // End time measurement
   /*struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time = (double) (end.tv_sec - start.tv_sec) + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
    printf("Time elapsed: %f seconds\n", time);*/

    double end = curtime();
    time = end - start;
    printf("time passed: %f\n", time);
}

void sobel_kernel_unroll(const uint8_t* img, size_t width, size_t height,
                         float a, float b, float c,
                         void* tmp,
                         uint8_t* result) {

    uint8_t* grayscale_image = (uint8_t*) tmp;
    img_to_grayscale(img, width, height, a, b, c, grayscale_image);


    uint8_t* image_0 = grayscale_image + width * 0;
    uint8_t* image_1 = grayscale_image + width * 1;
    uint8_t* image_2 = grayscale_image + width * 2;

    int row_indices[height];

    for (size_t i = 0; i < height; i++) {
        row_indices[i] = i * width;
    }

    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            int gx = 0, gy = 0;
            uint8_t top_left = (y > 0 && x > 0) ? image_0[x - 1] : 0;
            uint8_t top = (y > 0) ? image_0[x] : 0;
            uint8_t top_right = (y > 0 && x < width - 1) ? image_0[x + 1] : 0;
            uint8_t left = (x > 0) ? image_1[x - 1] : 0;
            uint8_t right = (x < width - 1) ? image_1[x + 1] : 0;
            uint8_t bottom_left = (y < height - 1 && x > 0) ? image_2[x - 1] : 0;
            uint8_t bottom = (y < height - 1) ? image_2[x] : 0;
            uint8_t bottom_right = (y < height - 1 && x < width - 1) ? image_2[x + 1] : 0;

            // Calculate gx and gy
            gx = top_left * (+1) + top_right * (-1) +
                 left * (+2) + right * (-2) +
                 bottom_left * (+1) + bottom_right * (-1);

            gy = top_left * (+1) + top * (+2) + top_right * (+1) +
                 bottom_left * (-1) + bottom * (-2) + bottom_right * (-1);

            int sum = gx * gx + gy * gy;
            uint8_t magnitude;
            if (sum < 65025) {
                magnitude = sqrt(sum);
            } else {
                magnitude = 255;
            }
            result[row_indices[y] + x] = magnitude;
        }
        if (y < height - 1) {
            image_0 += width;
            image_1 += width;
            image_2 += width;
        }
    }
}

void sobel_SIMD(const uint8_t* img, size_t width, size_t height,
            float a, float b, float c,
            void* tmp,
            uint8_t* result){

            uint8_t* grayscale_image = (uint8_t*)tmp;
            img_to_grayscale(img, width, height, a, b, c, grayscale_image);

            uint8_t* image_0 = grayscale_image + width * 0;
            uint8_t* image_1 = grayscale_image + width * 1;
            uint8_t* image_2 = grayscale_image + width * 2;

            __m128 const_p_one = _mm_set1_ps(+1.0f);
            __m128 const_p_two = _mm_set1_ps(+2.0f);
            __m128 const_n_one = _mm_set1_ps(-1.0f);
            __m128 const_n_two = _mm_set1_ps(-2.0f);

    double time = 0;
    double start = curtime();


        for (size_t y = 0; y < height; ++y) {
            for (size_t x = 0; x < width; x += 4) {
                // Handle zero padding at the edges
                __m128i current_0 = _mm_setzero_si128();
                __m128i current_1 = _mm_setzero_si128();
                __m128i current_2 = _mm_setzero_si128();

                if (y > 0) {
                    current_0 = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(image_0 + x - 1)), _mm_setzero_si128());
                }
                current_1 = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(image_1 + x - 1)), _mm_setzero_si128());
                if (y < height - 1) {
                    current_2 = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(image_2 + x - 1)), _mm_setzero_si128());
                }

                // Zero padding for columns
                if (x == 0) {
                    current_0 = _mm_srli_si128(current_0, 2);
                    current_1 = _mm_srli_si128(current_1, 2);
                    current_2 = _mm_srli_si128(current_2, 2);
                }

                if (x >= width - 4) {
                    current_0 = _mm_slli_si128(current_0, 2);
                    current_1 = _mm_slli_si128(current_1, 2);
                    current_2 = _mm_slli_si128(current_2, 2);
                }

                // Process Sobel filter
                __m128 image_00 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(current_0, _mm_setzero_si128()));
                __m128 image_01 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_srli_si128(current_0, 2), _mm_setzero_si128()));
                __m128 image_02 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_srli_si128(current_0, 4), _mm_setzero_si128()));
                __m128 image_10 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(current_1, _mm_setzero_si128()));
                __m128 image_12 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_srli_si128(current_1, 4), _mm_setzero_si128()));
                __m128 image_20 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(current_2, _mm_setzero_si128()));
                __m128 image_21 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_srli_si128(current_2, 2), _mm_setzero_si128()));
                __m128 image_22 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_srli_si128(current_2, 4), _mm_setzero_si128()));

                // Calculate gx and gy
                __m128 gx = _mm_add_ps(_mm_mul_ps(image_00, const_p_one),
                            _mm_add_ps(_mm_mul_ps(image_02, const_n_one),
                            _mm_add_ps(_mm_mul_ps(image_10, const_p_two),
                            _mm_add_ps(_mm_mul_ps(image_12, const_n_two),
                            _mm_add_ps(_mm_mul_ps(image_20, const_p_one),
                                   _mm_mul_ps(image_22, const_n_one))))));

                __m128 gy = _mm_add_ps(_mm_mul_ps(image_00, const_p_one),
                            _mm_add_ps(_mm_mul_ps(image_01, const_p_two),
                            _mm_add_ps(_mm_mul_ps(image_02, const_p_one),
                            _mm_add_ps(_mm_mul_ps(image_20, const_n_one),
                            _mm_add_ps(_mm_mul_ps(image_21, const_n_two),
                                   _mm_mul_ps(image_22, const_n_one))))));

                // Compute gradient magnitude
                __m128 gradient = _mm_sqrt_ps(_mm_add_ps(_mm_mul_ps(gx, gx), _mm_mul_ps(gy, gy)));
                __m128 clamped = _mm_min_ps(_mm_set1_ps(255.0f), gradient);
                __m128i pack_32 = _mm_cvtps_epi32(clamped);
                __m128i pack_16 = _mm_packus_epi32(pack_32, pack_32);
                __m128i pack_8 = _mm_packus_epi16(pack_16, pack_16);

                // Store the result
                _mm_storel_epi64((__m128i*)(result + y * width + x), pack_8);
            }

            image_0 += width;
            image_1 += width;
            image_2 += width;
        }

    double end = curtime();
    time = end - start;
    printf("time passed: %f\n", time);
}

void sobel_squareroot_lookup(const uint8_t* img, size_t width, size_t height,
                             float a, float b, float c,
                             void* tmp,
                             uint8_t* result) {

    printf("V2 Sobel implementation used.\n");

    // Temporary buffer for grayscale image
    uint8_t* grayscale_image = (uint8_t*)tmp;

    // Grayscale conversion
    img_to_grayscale(img, width, height, a, b, c, grayscale_image);
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


void sobel_separated_convolution(const uint8_t* img, size_t width, size_t height,
                                 float a, float b, float c,
                                 void* tmp,
                                 uint8_t* result) {

    printf("V4 Sobel implementation used.\n");

    // Temporary buffer for grayscale image
    uint8_t* grayscale_image = (uint8_t*)tmp;

    // Grayscale conversion
    img_to_grayscale(img, width, height, a, b, c, grayscale_image);

    size_t size = width * height;

    int *temporary_sum = malloc((size)*sizeof(int));
    int *temporary_sum_2 = malloc((size)*sizeof(int));
    int *temporary_sum_3 = malloc((size)*sizeof(int));

    int row_indices [height];

    double time = 0;
    double start = curtime();

    for (size_t i = 0; i < height; i++) {
        row_indices[i] = i * width;
    }

    int row = 0;
    int sum = 0;
    int sum2 = 0;

    for (size_t ppm_y = 0; ppm_y < height; ppm_y++) {
        for (size_t ppm_x = 0; ppm_x < width; ppm_x++) {
            sum = 0;
            sum2 = 0;
            row = row_indices[ppm_y];
            if (ppm_x > 0) {
                sum = grayscale_image[row + ppm_x - 1];
                sum2 = -grayscale_image[row + ppm_x - 1];
            }
            sum += grayscale_image[row + ppm_x] * 2;
            if (ppm_x + 1 < width) {
                sum += grayscale_image[row + ppm_x + 1];
                sum2 += grayscale_image[row + ppm_x + 1];
            }
            temporary_sum [row + ppm_x] = sum;
            temporary_sum_2 [row + ppm_x] = sum2;
        }
    }

    for (size_t ppm_y = 0; ppm_y < height; ppm_y++) {
        for (size_t ppm_x = 0; ppm_x < width; ppm_x++) {
            sum = 0;
            sum2 = 0;
            row = row_indices[ppm_y];
            if (ppm_y > 0) {
                sum = - temporary_sum[row - width + ppm_x];
                sum2 = temporary_sum_2[row - width + ppm_x];
            }
            sum2 += temporary_sum_2[row + ppm_x] * 2;
            if (ppm_y + 1 < height) {
                sum += temporary_sum[row + width + ppm_x];
                sum2 += temporary_sum_2[row + width + ppm_x];
            }
            temporary_sum_3 [row + ppm_x] = sum * sum + sum2 * sum2;
        }
    }

    for (size_t i = 0; i < size; i ++) {
        if (temporary_sum_3[i] < 65025) {
            result[i] = (uint8_t) sqrt(temporary_sum_3[i]);
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
