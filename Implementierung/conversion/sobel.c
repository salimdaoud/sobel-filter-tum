#define _POSIX_C_SOURCE 199309L
#include "sobel.h"
#include "grayscale.h"
#include <time.h>
#include <emmintrin.h>
#include <stddef.h>
#include <stdint.h>

// Sobel operator implementation
void sobel_naive( const uint8_t* img, size_t width, size_t height,
            float a, float b, float c,
            void* tmp,
            uint8_t* result, _Bool b_flag) {

    // Temporary buffer for grayscale image
    uint8_t* grayscale_image = (uint8_t*)tmp;

    //Grayscale calculation
    img_to_grayscale(img, width, height, a, b, c, grayscale_image, b_flag);

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
    if(b_flag) {
        clock_gettime(CLOCK_MONOTONIC, &start);
    }

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
    if(b_flag) {
        struct timespec end;
        clock_gettime(CLOCK_MONOTONIC, &end);
        double time = (double) (end.tv_sec - start.tv_sec) + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
        printf("Time elapsed for sobel_naive: %f seconds\n", time);
    }
}

void sobel_optimized( const uint8_t* img, size_t width, size_t height,
            float a, float b, float c,
            void* tmp,
            uint8_t* result, _Bool b_flag) {

            uint8_t* grayscale_image = (uint8_t*)tmp;
            img_to_grayscale(img, width, height, a, b, c, grayscale_image, b_flag);

            uint8_t* image_0 = grayscale_image + width * 0;
            uint8_t* image_1 = grayscale_image + width * 1;
            uint8_t* image_2 = grayscale_image + width * 2;

            //Start time measurement
            struct timespec start;
            if(b_flag) {
                clock_gettime(CLOCK_MONOTONIC, &start);
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
                    gx =    top_left * (+1) + top_right * (-1) +
                            left * (+2) + right * (-2) +
                            bottom_left * (+1) + bottom_right * (-1);

                    gy =    top_left * (+1) + top * (+2) + top_right * (+1) +
                            bottom_left * (-1) + bottom * (-2) + bottom_right * (-1);


                    int magnitude = (int) sqrt(gx * gx + gy * gy);
                    if (magnitude > 255) magnitude = 255;
                    result[y * width + x] = (uint8_t)magnitude;
                    }
                    if (y < height - 1) {
                        image_0 += width;
                        image_1 += width;
                        image_2 += width;
                        }
            }

            //End time measurement
            if(b_flag) {
                struct timespec end;
                clock_gettime(CLOCK_MONOTONIC, &end);
                double time = (double) (end.tv_sec - start.tv_sec) + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
                printf("Time elapsed for sobel_optimized: %f seconds\n", time);
            }
}

void sobel_SIMD(const uint8_t* img, size_t width, size_t height,
            float a, float b, float c,
            void* tmp,
            uint8_t* result, _Bool b_flag) {

    uint8_t *grayscale_image = (uint8_t *) tmp;
    img_to_grayscale(img, width, height, a, b, c, grayscale_image, b_flag);

    uint8_t *image_0 = grayscale_image + width * 0;
    uint8_t *image_1 = grayscale_image + width * 1;
    uint8_t *image_2 = grayscale_image + width * 2;

    __m128 const_p_one = _mm_set1_ps(+1.0f);
    __m128 const_p_two = _mm_set1_ps(+2.0f);
    __m128 const_n_one = _mm_set1_ps(-1.0f);
    __m128 const_n_two = _mm_set1_ps(-2.0f);

    // Start time measurement
    struct timespec start;
    if (b_flag) {
        clock_gettime(CLOCK_MONOTONIC, &start);
    }

    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; x += 4) {
            // Handle zero padding at the edges
            __m128i current_0 = _mm_setzero_si128();
            __m128i current_1 = _mm_setzero_si128();
            __m128i current_2 = _mm_setzero_si128();

            if (y > 0) {
                current_0 = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i *) (image_0 + x - 1)), _mm_setzero_si128());
            }
            current_1 = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i *) (image_1 + x - 1)), _mm_setzero_si128());
            if (y < height - 1) {
                current_2 = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i *) (image_2 + x - 1)), _mm_setzero_si128());
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
            _mm_storel_epi64((__m128i *) (result + y * width + x), pack_8);
        }

        image_0 += width;
        image_1 += width;
        image_2 += width;
    }

    // End time measurement
    if(b_flag) {
        struct timespec end;
        clock_gettime(CLOCK_MONOTONIC, &end);
        double time = (double) (end.tv_sec - start.tv_sec) + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
        printf("Time elapsed for sobel_SIMD: %f seconds\n", time);
    }
}