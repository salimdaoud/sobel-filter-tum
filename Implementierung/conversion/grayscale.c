#include "grayscale.h"

void img_to_grayscale_naive(const uint8_t* img, size_t width, size_t height,
                            float a, float b, float c, uint8_t* gray){
    for (size_t i = 0; i < width * height; i++) {
        size_t idx = i * 3; // Each pixel has 3 components (R, G, B)
        gray[i] = (uint8_t)((a * img[idx] + b * img[idx + 1] + c * img[idx + 2]) / (a + b + c));
    }
}

void img_to_grayscale_loop_unroll(const uint8_t* img, size_t width, size_t height, float a, float b, float c, uint8_t* gray){

    size_t rgb_values_count = width * height * 3;

    for (size_t i = 0, j = 0; i < rgb_values_count; i += 3, j++) {
        gray[j] = (uint8_t)(a * img[i] + b * img[i + 1] + c * img[i + 2]);
    }
}

void img_to_grayscale_bitshift(const uint8_t* img, size_t width, size_t height,
                               float a, float b, float c, uint8_t* gray){

    printf("WARNING: The grayscale coefficients are only roughly approximated: Red (0.25), Green (0.5)"
           " Blue (0.25).\nGiven coefficients for Red (%1.3f), Green (%1.3f) and Blue (%1.3f) will not be used.\n", a, b, c);

    size_t rgb_values_count = width * height * 3;

    for (size_t i = 0, j = 0; i < rgb_values_count; i += 3, j++) {
        gray[j] = (uint8_t)((img[i] >> 2) + (img[i + 1] >> 1) + (img[i + 2] >> 2));
    }
}

// Based on:    https://stackoverflow.com/questions/57832444/efficient-c-code-no-libs-
//              for-image-transformation-into-custom-rgb-pixel-grey/57844027#57844027
static __inline void extract_r_g_b_sorted(const __m128i r_5_bgr_4_bgr_3_bgr_2_bgr_1_bgr_0,
                                          const __m128i bgr_7_bgr_6_bgr_5_bgr_4,
                                          __m128i* r_76543210,
                                          __m128i* g_76543210,
                                          __m128i* b_76543210,
                                          const __m128i shuffle_mask);

static __inline __m128i convert_rgb_to_gray_8_pixels(__m128i r_76543210,
                                                     __m128i g_76543210,
                                                     __m128i b_76543210,
                                                     const __m128i r_coef_extended,
                                                     const __m128i g_coef_extended,
                                                     const __m128i b_coef_extended);

void img_to_grayscale_simd(const uint8_t* img, size_t width, size_t height,
                           float a, float b, float c, uint8_t* gray){
    size_t rgb_size = height * width * 3;

    // Each coefficient is expanded by 2^15 (not 2^16 as we use signed integers to be able to use _mm_mulhrs_epi16),
    // and rounded to int16 (add 0.5 for rounding). Cannot use shift because of float.
    // Fixed point multiplication allows to work on 8 16 bit integers instead of 4 32 bit floats. Requires special
    // handling of edge case where one coefficient is 1, as this would cause an overflow when shifting (multiplying
    // the float values) otherwise.
    int16_t r_value_weight_fixed = (int16_t)(a * 32768.0 + 0.5);
    int16_t g_value_weight_fixed = (int16_t)(b * 32768.0 + 0.5);
    int16_t b_value_weight_fixed = (int16_t)(c * 32768.0 + 0.5);

    const __m128i r_coef_extended = _mm_set1_epi16(r_value_weight_fixed);
    const __m128i g_coef_extended = _mm_set1_epi16(g_value_weight_fixed);
    const __m128i b_coef_extended = _mm_set1_epi16(b_value_weight_fixed);

    // Shuffle mask for gathering 4 Red Values, 4 Green Values and 4 Blue Values
    // (also set last 4 elements to duplication of first 4 elements).
    const __m128i shuffle_mask = _mm_set_epi8(9,6,3,0,      // red
                                              11,8,5,2,    // blue
                                              10,7,4,1,     // green
                                              9,6,3,0);  //red

    if (a > 0.999) {
        for (size_t i = 0, j = 0; i < rgb_size; i+= 3, j++) {
            gray[j] = (uint8_t) img[i];
        }
        return;
    } else if (b > 0.999) {
        for (size_t i = 1, j = 0; i < rgb_size; i+= 3, j++) {
            gray[j] = (uint8_t) img[i];
        }
        return;
    } else if (c > 0.999) {
        for (size_t i = 2, j = 0; i < rgb_size; i += 3, j++) {
            gray[j] = (uint8_t) img[i];
        }
        return;
    }

    //Process one row per iteration.
    for (size_t index_rgb = 0, index_gray = 0; index_rgb < rgb_size; index_rgb += 24, index_gray += 8) {

        __m128i r_76543210;
        __m128i g_76543210;
        __m128i b_76543210;

        // Load 8 elements of each color channel R,G,B from first row. Load 24 (8 * 3 values) unaligned
        // variable name is displayed as 128 bit integer. Within memory, values displayed in reversed order due to
        // Little-Endianness
        __m128i r_5_bgr_4_bgr_3_bgr_2_bgr_1_bgr_0 = _mm_loadu_si128((__m128i*)(img + index_rgb));
        __m128i bgr_7_bgr_6_bgr_5_bgr_4           = _mm_loadu_si128((__m128i*)(img + index_rgb + 12));

        // Separate RGB, and put together Red Values, Green Values and Blue Values (together in same XMM register).
        // Result is also unpacked from uint8 to uint16 elements.
        extract_r_g_b_sorted(r_5_bgr_4_bgr_3_bgr_2_bgr_1_bgr_0, bgr_7_bgr_6_bgr_5_bgr_4,
                         &r_76543210, &g_76543210, &b_76543210, shuffle_mask);

        // Calculate 8 Y elements.
        __m128i gray_16_76543210 = convert_rgb_to_gray_8_pixels(r_76543210, g_76543210, b_76543210,
                                                                r_coef_extended,
                                                                g_coef_extended,
                                                                b_coef_extended);

        // Pack uint16 elements to 16 uint8 elements (put result in single XMM register).
        // Only lower 8 uint8 elements are relevant.
        __m128i gray_8_76543210 = _mm_packus_epi16(gray_16_76543210, gray_16_76543210);

        // Store 8 elements of Y in row Y0, and 8 elements of Y in row Y1.
        _mm_storel_epi64((__m128i*)(gray + index_gray), gray_8_76543210);
    }
    // we do not need to handle the remaining pixels as we allocated more memory than necessary to have padding.
}

static __inline void extract_r_g_b_sorted(const __m128i r_5_bgr_4_bgr_3_bgr_2_bgr_1_bgr_0,
                                          const __m128i bgr_7_bgr_6_bgr_5_bgr_4,
                                          __m128i* r_76543210,
                                          __m128i* g_76543210,
                                          __m128i* b_76543210,
                                          const __m128i shuffle_mask){  // red

    // Gather 4 Red Values, 4 Green Values and 4 Blue Values. Order displayed as 128 bit integer. In memory, the data
    // lies in reversed order due to Little-Endianness.
    __m128i r_3210_b_3210_g_3210_r_3210 = _mm_shuffle_epi8(r_5_bgr_4_bgr_3_bgr_2_bgr_1_bgr_0, shuffle_mask);
    __m128i r_7654_b_7654_g_7654_r_7654 = _mm_shuffle_epi8(bgr_7_bgr_6_bgr_5_bgr_4, shuffle_mask);

    // RED: Put 8 Red Values in lower part.
    __m128i colour_7654_3210 = _mm_alignr_epi8(r_7654_b_7654_g_7654_r_7654, r_3210_b_3210_g_3210_r_3210, 12);
    // Unpack uint8 elements to uint16 elements.
    *r_76543210 = _mm_unpacklo_epi8(colour_7654_3210, _mm_setzero_si128());

    // GREEN: Put 8 Green Values in lower part.
    __m128i g_3210_r_3210_z_0000_z_0000 = _mm_slli_si128(r_3210_b_3210_g_3210_r_3210, 8);
    __m128i z_0000_r_7654_b_7654_g_7654 = _mm_srli_si128(r_7654_b_7654_g_7654_r_7654, 4);
    colour_7654_3210 = _mm_alignr_epi8(z_0000_r_7654_b_7654_g_7654, g_3210_r_3210_z_0000_z_0000, 12);
    // Unpack uint8 elements to uint16 elements.
    *g_76543210 = _mm_unpacklo_epi8(colour_7654_3210, _mm_setzero_si128());

    // BLUE: Put 8 Blue Values in lower part.
    __m128i b_3210_g_3210_r_3210_z_0000 = _mm_slli_si128(r_3210_b_3210_g_3210_r_3210, 4);
    __m128i z_0000_z_0000_r_7654_b_7654 = _mm_srli_si128(r_7654_b_7654_g_7654_r_7654, 8);
    colour_7654_3210 = _mm_alignr_epi8(z_0000_z_0000_r_7654_b_7654, b_3210_g_3210_r_3210_z_0000, 12);
    // Unpack uint8 elements to uint16 elements.
    *b_76543210 = _mm_unpacklo_epi8(colour_7654_3210, _mm_setzero_si128());
}

// Calculate 8 Grayscale elements from 8 RGB Values.
static __inline __m128i convert_rgb_to_gray_8_pixels(__m128i r_76543210, __m128i g_76543210, __m128i b_76543210,
                                                     const __m128i r_coef_extended,
                                                     const __m128i g_coef_extended,
                                                     const __m128i b_coef_extended) {


    // Multiply input elements by 64 for improved accuracy.
    r_76543210 = _mm_slli_epi16(r_76543210, 6);
    g_76543210 = _mm_slli_epi16(g_76543210, 6);
    b_76543210 = _mm_slli_epi16(b_76543210, 6);

    // Use the special intrinsic _mm_mulhrs_epi16 that calculates round((r * r_coef) / 2^15).
    // Calculate gray_76543210 using fixed point multiplication and adding up the results.
    __m128i gray_76543210 = _mm_add_epi16(_mm_add_epi16( _mm_mulhrs_epi16(r_76543210, r_coef_extended),
                                                            _mm_mulhrs_epi16(g_76543210, g_coef_extended)),
                                                            _mm_mulhrs_epi16(b_76543210, b_coef_extended));

    // Divide result by 64 to redo the accuracy related multiplication above.
    gray_76543210 = _mm_srli_epi16(gray_76543210, 6);

    return gray_76543210;
}

// Tested if compiler optimizations would be more effective with less shifts and without alignr. But Grayscaling
// with 8 pixels was still faster.
static __inline void extract_r_g_b_sorted_5_pixels(const __m128i r_5_bgr_4_bgr_3_bgr_2_bgr_1_bgr_0,
                                                   __m128i* r_43210,
                                                   __m128i* g_43210,
                                                   __m128i* b_43210);

static __inline __m128i convert_rgb_to_gray_5_pixels(__m128i r_43210,
                                                     __m128i g_43210,
                                                     __m128i b_43210,
                                                     const __m128i r_coef_extended,
                                                     const __m128i g_coef_extended,
                                                     const __m128i b_coef_extended);

void img_to_grayscale_simd_5_pixels(const uint8_t* img, size_t width, size_t height,
                                    float a, float b, float c, uint8_t* gray){
    size_t rgb_size = height * width * 3;
    size_t simd_size = rgb_size - (rgb_size % 24);
    size_t index_rgb = 0;
    size_t index_gray = 0;

    // Each coefficient is expanded by 2^15 (not 2^16 as we use signed integers to be able to use _mm_mulhrs_epi16),
    // and rounded to int16 (add 0.5 for rounding). Cannot use shift because of float.
    int16_t r_value_weight_fixed = (int16_t)(a * 32768.0 + 0.5);
    int16_t g_value_weight_fixed = (int16_t)(b * 32768.0 + 0.5);
    int16_t b_value_weight_fixed = (int16_t)(c * 32768.0 + 0.5);

    const __m128i r_coef_extended = _mm_set_epi16(0, 0, 0,
                                                  r_value_weight_fixed, r_value_weight_fixed,
                                                  r_value_weight_fixed, r_value_weight_fixed, r_value_weight_fixed);
    const __m128i g_coef_extended = _mm_set_epi16(0,0,0,
                                                  g_value_weight_fixed, g_value_weight_fixed,
                                                  g_value_weight_fixed,g_value_weight_fixed,g_value_weight_fixed);
    const __m128i b_coef_extended = _mm_set_epi16(0, 0, 0,
                                                  b_value_weight_fixed,b_value_weight_fixed,b_value_weight_fixed,
                                                  b_value_weight_fixed,b_value_weight_fixed);

    // Process one row per iteration.
    for (; index_rgb < simd_size; index_rgb += 15, index_gray += 5) {

        __m128i r_43210;
        __m128i g_43210;
        __m128i b_43210;

        // Load 8 elements of each color channel R,G,B from first row. Load 15 (5 * 3 values) unaligned
        // variable name is displayed as 128 bit integer. Within memory, values displayed in reversed order due to
        // Little-Endianness
        __m128i r_5_bgr_4_bgr_3_bgr_2_bgr_1_bgr_0 = _mm_loadu_si128((__m128i*)(img + index_rgb));

        // Separate RGB, and put together Red Values, Green Values and Blue Values (together in same XMM register).
        // Result is also unpacked from uint8 to uint16 elements.
        extract_r_g_b_sorted_5_pixels(r_5_bgr_4_bgr_3_bgr_2_bgr_1_bgr_0,
                                      &r_43210, &g_43210, &b_43210);

        __m128i gray_5_43210;

        if (a > 0.999) {
            gray_5_43210 = _mm_packus_epi16(r_43210, r_43210);
        } else if (b > 0.999) {
            gray_5_43210 = _mm_packus_epi16(g_43210, g_43210);
        } else if (c > 0.999) {
            gray_5_43210 = _mm_packus_epi16(b_43210, b_43210);
        } else {
            // Calculate 8 Y elements.
            __m128i gray_16_76543210 = convert_rgb_to_gray_5_pixels(r_43210, g_43210, b_43210,
                                                                    r_coef_extended,
                                                                    g_coef_extended,
                                                                    b_coef_extended);

            // Pack uint16 elements to 16 uint8 elements (put result in single XMM register).
            // Only lower 8 uint8 elements are relevant.
            gray_5_43210 = _mm_packus_epi16(gray_16_76543210, gray_16_76543210);
        }

        // Store 8 elements of Y in row Y0, and 8 elements of Y in row Y1.
        _mm_storel_epi64((__m128i*)(gray + index_gray), gray_5_43210);
    }

    // Handle the rest of the pixels. Naive implementation as there are at most 23 values.
    size_t pixels_left = rgb_size % 15;

    for (size_t i = 0;  i < pixels_left; index_gray++, i += 3) {
        size_t idx = index_rgb + i;
        gray[index_gray] = (uint8_t)(a * img[idx] + b * img[idx + 1] + c * img[idx + 2]);
    }
}

static __inline void extract_r_g_b_sorted_5_pixels(const __m128i r_5_bgr_4_bgr_3_bgr_2_bgr_1_bgr_0,
                                                   __m128i* r_43210,
                                                   __m128i* g_43210,
                                                   __m128i* b_43210){

    // Shuffle mask for gathering 5 Red Values, 5 Green Values and 5 Blue Values (+ 1 filler)
    const __m128i shuffle_mask = _mm_set_epi8(15,                                // filler
                                              14, 11,8,5,2,  // blue
                                              13, 10,7,4,1, //green
                                              12, 9,6,3,0); //red

    // Gather 5 Red Values, 5 Green Values and 5 Blue Values. Order displayed as 128 bit integer. In memory, the data
    // lies in reversed order due to Little-Endianness.
    __m128i r_0_b_43210_g_43210_r_43210 = _mm_shuffle_epi8(r_5_bgr_4_bgr_3_bgr_2_bgr_1_bgr_0, shuffle_mask);

    // RED: Put 5 Blue Values in lower part.
    *r_43210 = _mm_unpacklo_epi8(r_0_b_43210_g_43210_r_43210, _mm_setzero_si128());

    // GREEN: Put 5 Blue Values in lower part.
    __m128i unclean_colour_43210 = _mm_srli_si128(r_0_b_43210_g_43210_r_43210, 5);
    *g_43210 = _mm_unpacklo_epi8(unclean_colour_43210, _mm_setzero_si128());

    // BLUE: Put 5 Blue Values in lower part.
    unclean_colour_43210 = _mm_srli_si128(unclean_colour_43210, 5);
    *b_43210 = _mm_unpacklo_epi8(unclean_colour_43210, _mm_setzero_si128());
}

// Calculate 5 Grayscale elements from 5 RGB Values.
static __inline __m128i convert_rgb_to_gray_5_pixels(__m128i r_43210, __m128i g_43210, __m128i b_43210,
                                                     const __m128i r_coef_extended,
                                                     const __m128i g_coef_extended,
                                                     const __m128i b_coef_extended) {


    // Multiply input elements by 64 for improved accuracy.
    r_43210 = _mm_slli_epi16(r_43210, 6);
    g_43210 = _mm_slli_epi16(g_43210, 6);
    b_43210 = _mm_slli_epi16(b_43210, 6);

    // Use the special intrinsic _mm_mulhrs_epi16 that calculates round((r * r_coef) / 2^15).
    // Calculate gray_43210 using fixed point multiplication and adding up the results.
    __m128i gray_43210 = _mm_add_epi64(_mm_add_epi64(_mm_mulhrs_epi16(r_43210, r_coef_extended),
                                                     _mm_mulhrs_epi16(g_43210, g_coef_extended)),
                                       _mm_mulhrs_epi16(b_43210, b_coef_extended));

    // Divide result by 64 to redo the accuracy related multiplication above.
    gray_43210 = _mm_srli_epi16(gray_43210, 6);

    return gray_43210;
}