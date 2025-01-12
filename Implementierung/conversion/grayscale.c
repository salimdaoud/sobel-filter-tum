#include "grayscale.h"

void img_to_grayscale_naive(const uint8_t* img, size_t width, size_t height,
               float a, float b, float c, uint8_t* gray){


    for (size_t i = 0; i < width * height; i++) {
        size_t idx = i * 3; // Each pixel has 3 components (R, G, B)
        gray[i] = (uint8_t)((a * img[idx] + b * img[idx + 1] + c * img[idx + 2]) / (a + b + c));
    }

}

void img_to_grayscale_SIMD(const uint8_t* img, size_t width, size_t height,
                           float a, float b, float c, uint8_t* gray){
    size_t total_pixels = width * height;
    __m128 weight_r = _mm_set1_ps(a);        // [a, a, a, a]
    __m128 weight_g = _mm_set1_ps(b);        // [b, b, b, b]
    __m128 weight_b = _mm_set1_ps(c);        // [c, c, c, c]
    size_t i = 0;
    size_t simd_pixels = total_pixels - (total_pixels % 4); // Process in chunks of 4 pixels

    for (; i < simd_pixels; i+=4){
        __m128 red = _mm_set_ps(
                (float)img[i * 3 + 9], (float)img[i * 3 + 6],
                (float)img[i * 3 + 3], (float)img[i * 3]);
        __m128 blue = _mm_set_ps(
                (float)img[i * 3 + 10], (float)img[i * 3 + 7],
                (float)img[i * 3 + 4], (float)img[i * 3 + 1]);
        __m128 green = _mm_set_ps(
                (float)img[i * 3 + 11], (float)img[i * 3 + 8],
                (float)img[i * 3 + 5], (float)img[i * 3 + 2]);

        __m128 weighted_red = _mm_mul_ps(red, weight_r);
        __m128 weighted_blue = _mm_mul_ps(blue, weight_g);
        __m128 weighted_green = _mm_mul_ps(green, weight_b);

        // Convert floating-point grayscale values to integers
        __m128i grayscale_int = _mm_cvttps_epi32(_mm_add_ps(weighted_red, _mm_add_ps(weighted_blue, weighted_green))); // [G1, G2, G3, G4]

        // Extract the grayscale values and store them
        gray[i + 0] = (uint8_t)_mm_extract_epi16(grayscale_int, 0);
        gray[i + 1] = (uint8_t)_mm_extract_epi16(grayscale_int, 2);
        gray[i + 2] = (uint8_t)_mm_extract_epi16(grayscale_int, 4);
        gray[i + 3] = (uint8_t)_mm_extract_epi16(grayscale_int, 6);
    }

    // Handle remaining pixels (if total_pixels is not divisible by 4)
    for (; i < total_pixels; i++) {
        size_t idx = i * 3; // Each pixel has 3 components (R, G, B)
        gray[i] = (uint8_t)((a * img[idx] + b * img[idx + 1] + c * img[idx + 2]) / (a + b + c));
    }
}

void img_to_grayscale(const uint8_t* img, size_t width, size_t height, float a, float b, float c, uint8_t* gray){

    size_t rgb_values_count = width * height * 3;

    for (size_t i = 0, j = 0; i < rgb_values_count; i += 3, j++) {
        gray[j] = (uint8_t)(a * img[i] + b * img[i + 1] + c * img[i + 2]);
    }
}

void img_to_grayscale_bitshift(const uint8_t* img, size_t width, size_t height,
                               float a, float b, float c, uint8_t* gray){

    size_t rgb_values_count = width * height * 3;

    for (size_t i = 0, j = 0; i < rgb_values_count; i += 3, j++) {
        gray[j] = (uint8_t)((img[i] >> 2) + (img[i + 1] >> 1) + (img[i + 2] >> 2));
    }
}




