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
    __m128 weight_r = _mm_set1_ps(a);       // [a, a, a, a]
    __m128 weight_g = _mm_set1_ps(b);        // [b, b, b, b]
    __m128 weight_b = _mm_set1_ps(c);        // [c, c, c, c]
    size_t i = 0;
    size_t simd_pixels = total_pixels - (total_pixels % 4); // Process in chunks of 4 pixels

    for (; i < simd_pixels; i+=4){
        size_t i_times_three = i * 3;
        __m128 red = _mm_set_ps(
                (float)img[i_times_three + 9], (float)img[i_times_three + 6],
                (float)img[i_times_three + 3], (float)img[i_times_three]);
        __m128 blue = _mm_set_ps(
                (float)img[i_times_three + 10], (float)img[i_times_three + 7],
                (float)img[i_times_three + 4], (float)img[i_times_three + 1]);
        __m128 green = _mm_set_ps(
                (float)img[i_times_three + 11], (float)img[i_times_three + 8],
                (float)img[i_times_three + 5], (float)img[i_times_three + 2]);

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

//void img_to_grayscale_SIMD_optimized(const uint8_t* img, size_t width, size_t height,
//                           float a, float b, float c, uint8_t* gray){
//    size_t total_pixels = width * height;
//    __m128 weight_vector_1 = _mm_set_ps(0x3e991687, 0x3de978d5, 0x3f1645a2, 0x3e991687); // [r, g, b, r]
//    __m128 weight_vector_2 = _mm_set_ps(0x3f1645a2, 0x3e991687, 0x3de978d5, 0x3f1645a2); // [g, b, r, g]
//    __m128 weight_vector_3 = _mm_set_ps(0x3de978d5, 0x3f1645a2, 0x3e991687, 0x3de978d5); // [b, r, g, b]
//    size_t i = 0;
//    size_t simd_pixels = total_pixels - (total_pixels % 4); // Process in chunks of 4 pixels
//
//    for (; i < simd_pixels; i+=4){
//        __m128 sixteen_byte_from_pointer = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)img),
//                                                             _mm_setzero_si128());
//
//
//
//
//
//
//
//
//        if (gray_y > 0) {
//            top_row = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(image_row_prev + gray_x - 1)),
//                                        _mm_setzero_si128());
//        }
//        current_row = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(image_row_now + gray_x - 1)),
//                                        _mm_setzero_si128());
//        bot_row = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(image_row_next + gray_x - 1)),
//                                    _mm_setzero_si128());
//
//        // Process Sobel filter
//        __m128i top_left =  _mm_unpacklo_epi16(top_row, _mm_setzero_si128());
//        __m128i top =       _mm_unpacklo_epi16(_mm_srli_si128(top_row, 2), _mm_setzero_si128());
//        __m128i top_right = _mm_unpacklo_epi16(_mm_srli_si128(top_row, 4), _mm_setzero_si128());
//        __m128i left =      _mm_unpacklo_epi16(current_row, _mm_setzero_si128());
//        __m128i right =     _mm_unpacklo_epi16(_mm_srli_si128(current_row, 4), _mm_setzero_si128());
//
//
//
//        __m128 red = _mm_set_ps(
//                (float)img[i * 3 + 9], (float)img[i * 3 + 6],
//                (float)img[i * 3 + 3], (float)img[i * 3]);
//        __m128 blue = _mm_set_ps(
//                (float)img[i * 3 + 10], (float)img[i * 3 + 7],
//                (float)img[i * 3 + 4], (float)img[i * 3 + 1]);
//        __m128 green = _mm_set_ps(
//                (float)img[i * 3 + 11], (float)img[i * 3 + 8],
//                (float)img[i * 3 + 5], (float)img[i * 3 + 2]);
//
//        __m128 weighted_red = _mm_mul_ps(red, weight_r);
//        __m128 weighted_blue = _mm_mul_ps(blue, weight_g);
//        __m128 weighted_green = _mm_mul_ps(green, weight_b);
//
//        // Convert floating-point grayscale values to integers
//        __m128i grayscale_int = _mm_cvttps_epi32(_mm_add_ps(weighted_red, _mm_add_ps(weighted_blue, weighted_green))); // [G1, G2, G3, G4]
//
//        // Extract the grayscale values and store them
//        gray[i + 0] = (uint8_t)_mm_extract_epi16(grayscale_int, 0);
//        gray[i + 1] = (uint8_t)_mm_extract_epi16(grayscale_int, 2);
//        gray[i + 2] = (uint8_t)_mm_extract_epi16(grayscale_int, 4);
//        gray[i + 3] = (uint8_t)_mm_extract_epi16(grayscale_int, 6);
//    }
//
//    // Handle remaining pixels (if total_pixels is not divisible by 4)
//    for (; i < total_pixels; i++) {
//        size_t idx = i * 3; // Each pixel has 3 components (R, G, B)
//        gray[i] = (uint8_t)((a * img[idx] + b * img[idx + 1] + c * img[idx + 2]) / (a + b + c));
//    }
//}

void img_to_grayscale(const uint8_t* img, size_t width, size_t height, float a, float b, float c, uint8_t* gray){

    size_t rgb_values_count = width * height * 3;

    for (size_t i = 0, j = 0; i < rgb_values_count; i += 3, j++) {
        gray[j] = (uint8_t)(a * img[i] + b * img[i + 1] + c * img[i + 2]);
    }
}

void img_to_grayscale_bitshift(const uint8_t* img, size_t width, size_t height,
                               float a, float b, float c, uint8_t* gray){

    printf("WARNING: The grayscale coefficients are only roughly approximated: Red (0.25), Green (0.5)+"
           " Blue (0.25).\nGiven coefficients for Red (%1.3f), Green (%1.3f) and Blue (%1.3f) will not be used.\n", a, b, c);

    size_t rgb_values_count = width * height * 3;

    for (size_t i = 0, j = 0; i < rgb_values_count; i += 3, j++) {
        gray[j] = (uint8_t)((img[i] >> 2) + (img[i + 1] >> 1) + (img[i + 2] >> 2));
    }
}

// Source: https://stackoverflow.com/questions/57832444/efficient-c-code-no-libs-for-image-transformation-into-custom-rgb-pixel-grey/57844027#57844027
static __inline void extract_r_g_b_sorted(const __m128i r_5_bgr_4_bgr_3_bgr_2_bgr_1_bgr_0,
                                          const __m128i bgr_7_bgr_6_bg_5,
                                          __m128i* r_76543210,
                                          __m128i* g_76543210,
                                          __m128i* b_76543210);

static __inline __m128i convert_rgb_to_gray_8_pixels(__m128i r_76543210,
                                                     __m128i g_76543210,
                                                     __m128i b_76543210,
                                                     float r_value_weighted,
                                                     float g_value_weighted,
                                                     float b_value_weighted);


void img_to_grayscale_simd_8_pixels(const uint8_t* img, size_t width, size_t height,
                                    float a, float b, float c, uint8_t* gray){
    size_t rgb_size = height * width * 3;
    size_t simd_size = rgb_size - (rgb_size % 24);
    size_t index_rgb = 0;
    size_t index_gray = 0;

    //Process one row per iteration.
    for (; index_rgb < simd_size; index_rgb += 24, index_gray += 8) {

        __m128i r_76543210;
        __m128i g_76543210;
        __m128i b_76543210;
        // Load 8 elements of each color channel R,G,B from first row. Load 24 (8 * 3 values) unaligned, 16 + 8 char
        __m128i r_5_bgr_4_bgr_3_bgr_2_bgr_1_bgr_0 = _mm_loadu_si128((__m128i*)(img + index_rgb));
        __m128i bgr_7_bgr_6_bg_5                  = _mm_loadu_si128((__m128i*)(img + index_rgb + 16));

        // Separate RGB, and put together R elements, G elements and B elements (together in same XMM register).
        // Result is also unpacked from uint8 to uint16 elements.
        extract_r_g_b_sorted(r_5_bgr_4_bgr_3_bgr_2_bgr_1_bgr_0, bgr_7_bgr_6_bg_5,
                             &r_76543210, &g_76543210, &b_76543210);

        // Calculate 8 Y elements.
        __m128i gray_16_76543210 = convert_rgb_to_gray_8_pixels(r_76543210, g_76543210, b_76543210,
                                                                a, b, c);

        // Pack uint16 elements to 16 uint8 elements (put result in single XMM register). Only lower 8 uint8 elements are relevant.
        __m128i gray_8_76543210 = _mm_packus_epi16(gray_16_76543210, gray_16_76543210);

        // Store 8 elements of Y in row Y0, and 8 elements of Y in row Y1.
        _mm_storel_epi64((__m128i*)(gray + index_gray), gray_8_76543210);
    }

    size_t pixels_left = rgb_size % 24;

    for (size_t i = 0;  i < pixels_left; index_gray++, i += 3) {
        size_t idx = index_rgb + i; // Each pixel has 3 components (R, G, B)
        gray[index_gray] = (uint8_t)(a * img[idx] + b * img[idx + 1] + c * img[idx + 2]);
    }
}

static __inline void extract_r_g_b_sorted(const __m128i r_5_bgr_4_bgr_3_bgr_2_bgr_1_bgr_0,
                                          const __m128i bgr_7_bgr_6_bg_5,
                                          __m128i* r_76543210,
                                          __m128i* g_76543210,
                                          __m128i* b_76543210){

//Shuffle mask for gathering 4 R elements, 4 G elements and 4 B elements (also set last 4 elements to duplication of first 4 elements).
const __m128i shuffle_mask = _mm_set_epi8(9,6,3,0,      // red
                                          11,8,5,2,     // green
                                          10,7,4,1,     // blue
                                          9,6,3,0);     // red

// move missing rgb values from 16 element register to former 8 element register.
__m128i bgr7_bgr6_bgr5_bgr_4 = _mm_alignr_epi8(bgr_7_bgr_6_bg_5, r_5_bgr_4_bgr_3_bgr_2_bgr_1_bgr_0, 12);

// Gather 4 R elements, 4 G elements and 4 B elements.
__m128i r_3210_b_3210_g_3210_r_3210 = _mm_shuffle_epi8(r_5_bgr_4_bgr_3_bgr_2_bgr_1_bgr_0, shuffle_mask);
__m128i r_7654_b7654_g_7654_r_7654 = _mm_shuffle_epi8(bgr7_bgr6_bgr5_bgr_4, shuffle_mask);

    // RED: Put 8 R elements in lower part.
    __m128i b_7654_g_7654_r_7654_r_3210 = _mm_alignr_epi8(r_7654_b7654_g_7654_r_7654, r_3210_b_3210_g_3210_r_3210, 12);
    
    // GREEN: Put 8 G elements in lower part.
    __m128i g_3210_b_3210_z_0000_z_0000 = _mm_slli_si128(r_3210_b_3210_g_3210_r_3210, 8);
    __m128i z_0000_r_7654_b_7654_g_7654 = _mm_srli_si128(r_7654_b7654_g_7654_r_7654, 4);
    __m128i r_7654_b_7654_g_7654_g_3210 = _mm_alignr_epi8(z_0000_r_7654_b_7654_g_7654, g_3210_b_3210_z_0000_z_0000, 12);
    
    // BLUE: Put 8 B elements in lower part.
    __m128i b_3210_g_3210_r_3210_z_0000 = _mm_slli_si128(r_3210_b_3210_g_3210_r_3210, 4);
    __m128i z_0000_z_0000_r_7654_b_7654 = _mm_srli_si128(r_7654_b7654_g_7654_r_7654, 8);
    __m128i z_0000_r_7654_b_7654_b_3210 = _mm_alignr_epi8(z_0000_z_0000_r_7654_b_7654, b_3210_g_3210_r_3210_z_0000, 12);

// Unpack uint8 elements to uint16 elements.
*r_76543210 = _mm_cvtepu8_epi16(b_7654_g_7654_r_7654_r_3210);
*g_76543210 = _mm_cvtepu8_epi16(r_7654_b_7654_g_7654_g_3210);
*b_76543210 = _mm_cvtepu8_epi16(z_0000_r_7654_b_7654_b_3210);
}

// Calculate 8 Grayscale elements from 8 RGB elements.
static __inline __m128i convert_rgb_to_gray_8_pixels(__m128i r_76543210, __m128i g_76543210, __m128i b_76543210,
                                                     float r_value_weighted, float g_value_weighted, float b_value_weighted) {

    // Each coefficient is expanded by 2^15 (not 2^16 as we use signed integers to be able to use _mm_mulhrs_epi16),
    // and rounded to int16 (add 0.5 for rounding). Cannot use shift because of float.
    const __m128i r_coef_extended = _mm_set1_epi16((int16_t)(r_value_weighted * 32768.0 + 0.5));
    const __m128i g_coef_extended = _mm_set1_epi16((int16_t)(g_value_weighted * 32768.0 + 0.5));
    const __m128i b_coef_extended = _mm_set1_epi16((int16_t)(b_value_weighted * 32768.0 + 0.5));

    // Multiply input elements by 64 for improved accuracy.
    r_76543210 = _mm_slli_epi16(r_76543210, 6);
    g_76543210 = _mm_slli_epi16(g_76543210, 6);
    b_76543210 = _mm_slli_epi16(b_76543210, 6);

    // Use the special intrinsic _mm_mulhrs_epi16 that calculates round(r*r_coef/2^15).
    // Calculate gray_value = 0.2989*R + 0.5870*G + 0.1140*B (use fixed point computations)
    __m128i gray_76543210 = _mm_add_epi16(_mm_add_epi16( _mm_mulhrs_epi16(r_76543210, r_coef_extended),
                                                            _mm_mulhrs_epi16(g_76543210, g_coef_extended)),
                                                            _mm_mulhrs_epi16(b_76543210, b_coef_extended));

    // Divide result by 64 to redo the multiplication above.
    gray_76543210 = _mm_srli_epi16(gray_76543210, 6);

    return gray_76543210;
}