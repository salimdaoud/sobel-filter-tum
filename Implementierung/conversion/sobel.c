#include "sobel.h"

// Naive approach
void sobel(const uint8_t* img, size_t width, size_t height,
           float a, float b, float c, void* tmp, uint8_t* result) {

    // Temporary buffer for grayscale image
    uint8_t* grayscale_image = (uint8_t*)tmp;

    img_to_grayscale_naive(img, width, height, a, b, c, grayscale_image);

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

    for (size_t gray_y = 0; gray_y < height; gray_y++) {
        for (size_t gray_x = 0; gray_x < width; gray_x++) {
            int sum_vertical = 0, sum_horizontal = 0;

            // Apply Sobel kernel to pixel
            for (int kern_y = -1; kern_y <= 1; kern_y++) {
                // Only apply kernel values within ppm grid
                if ((gray_y + kern_y != SIZE_MAX) && (gray_y + kern_y < height)) {
                    for (int kern_x = -1; kern_x <= 1; kern_x++) {
                        // Only apply kernel values within ppm grid
                        if ((gray_x + kern_x != SIZE_MAX) && (gray_x + kern_x < width)){
                            uint8_t pixel = grayscale_image[(kern_y + gray_y) * width + (gray_x + kern_x)];
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
            result[gray_y * width + gray_x] = (uint8_t)magnitude;
        }
    }
}

// Kernel Unroll
void sobel_V1(const uint8_t* img, size_t width, size_t height,
              float a, float b, float c, void* tmp, uint8_t* result) {

    uint8_t* grayscale_image = (uint8_t*) tmp;
    img_to_grayscale_loop_unroll(img, width, height, a, b, c, grayscale_image);

    uint8_t* image_row_prev = grayscale_image - width;
    uint8_t* image_row_now = grayscale_image;
    uint8_t* image_row_next = grayscale_image + width;

    size_t row_indices[height];

    for (size_t i = 0; i < height; i++) {
        row_indices[i] = i * width;
    }

    for (size_t gray_y = 0; gray_y < height; gray_y++) {
        for (size_t gray_x = 0; gray_x < width; gray_x++) {
            int gradient_vert = 0, gradient_hor = 0;
            size_t last_row = height - 1;
            size_t last_column = width - 1;
            uint8_t top_left =  (gray_y > 0 && gray_x > 0) ? image_row_prev[gray_x - 1] : 0;
            uint8_t top =       (gray_y > 0) ? image_row_prev[gray_x] : 0;
            uint8_t top_right = (gray_y > 0 && gray_x < last_column) ? image_row_prev[gray_x + 1] : 0;
            uint8_t left =      (gray_x > 0) ? image_row_now[gray_x - 1] : 0;
            uint8_t right =     (gray_x < last_column) ? image_row_now[gray_x + 1] : 0;
            uint8_t bot_left =  (gray_y < last_row && gray_x > 0) ? image_row_next[gray_x - 1] : 0;
            uint8_t bottom =    (gray_y < last_row) ? image_row_next[gray_x] : 0;
            uint8_t bot_right = (gray_y < last_row && gray_x < last_column) ? image_row_next[gray_x + 1] : 0;

            // Calculate gradient_vert and gradient_hor
            gradient_vert = top_left + left + left + bot_left
                            - top_right - right - right - bot_right;

            gradient_hor = top_left + top + top + top_right
                            - bot_left - bottom - bottom - bot_right;

            int sum = gradient_vert * gradient_vert + gradient_hor * gradient_hor;
            uint8_t magnitude;
            if (sum < 65025) {
                magnitude = sqrt(sum);
            } else {
                magnitude = 255;
            }
            result[row_indices[gray_y] + gray_x] = magnitude;
        }
        image_row_prev += width;
        image_row_now += width;
        image_row_next += width;
    }
}

// SIMD
void sobel_V3(const uint8_t* img, size_t width, size_t height,
              float a, float b, float c, void* tmp, uint8_t* result) {

    uint8_t* grayscale_image = (uint8_t*) tmp;
    img_to_grayscale_simd_5_pixels(img, width, height, a, b, c, grayscale_image);

    // Store 1 set of 8 values for each row of the Sobel Matrix.
    uint8_t* image_row_prev = grayscale_image - width;
    uint8_t* image_row_now = grayscale_image;
    uint8_t* image_row_next = grayscale_image + width;

    size_t row_offset = 0;
    // The remaining pixels of each row which cannot be handled without restrictions. Even if the width can be divided
    // by 8, loading the 8 right values would load 1 incorrect value, which is why they also need to be masked.
    uint8_t remaining_pixels_row = width % 8 == 0 ? 8 : (width % 8);
    size_t border_pixels_row = width - remaining_pixels_row;
    __m128i zero_mask_left_pixels;
    __m128i zero_mask_mid_pixels;
    __m128i zero_mask_right_pixels;
    __m128i zero_mask_drop_first_1_word = _mm_set_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFF0000);
    __m128i max_value_extended = _mm_set1_epi32(255);

    switch (width % 8) {
        case 0:
            zero_mask_left_pixels = _mm_set_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
            zero_mask_mid_pixels = _mm_set_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
            zero_mask_right_pixels = _mm_set_epi32(0x0000FFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
            break;
        case 7:
            zero_mask_left_pixels = _mm_set_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
            zero_mask_mid_pixels = _mm_set_epi32(0x0000FFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
            zero_mask_right_pixels = _mm_set_epi32(0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
            break;
        case 6:
            zero_mask_left_pixels = _mm_set_epi32(0x0000FFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
            zero_mask_mid_pixels = _mm_set_epi32(0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
            zero_mask_right_pixels = _mm_set_epi32(0x00000000, 0x0000FFFF, 0xFFFFFFFF, 0xFFFFFFFF);
            break;
        case 5:
            zero_mask_left_pixels = _mm_set_epi32(0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
            zero_mask_mid_pixels = _mm_set_epi32(0x00000000, 0x0000FFFF, 0xFFFFFFFF, 0xFFFFFFFF);
            zero_mask_right_pixels = _mm_set_epi32(0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF);
            break;
        case 4:
            zero_mask_left_pixels = _mm_set_epi32(0x00000000, 0x0000FFFF, 0xFFFFFFFF, 0xFFFFFFFF);
            zero_mask_mid_pixels = _mm_set_epi32(0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF);
            zero_mask_right_pixels = _mm_set_epi32(0x00000000, 0x00000000, 0x0000FFFF, 0xFFFFFFFF);
            break;
        case 3:
            zero_mask_left_pixels = _mm_set_epi32(0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF);
            zero_mask_mid_pixels = _mm_set_epi32(0x00000000, 0x00000000, 0x0000FFFF, 0xFFFFFFFF);
            zero_mask_right_pixels = _mm_set_epi32(0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF);
            break;
        case 2:
            zero_mask_left_pixels = _mm_set_epi32(0x00000000, 0x00000000, 0x0000FFFF, 0xFFFFFFFF);
            zero_mask_mid_pixels = _mm_set_epi32(0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF);
            zero_mask_right_pixels = _mm_set_epi32(0x00000000, 0x00000000, 0x00000000, 0x0000FFFF);
            break;
        case 1:
            zero_mask_left_pixels = _mm_set_epi32(0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF);
            zero_mask_mid_pixels = _mm_set_epi32(0x00000000, 0x00000000, 0x00000000, 0x0000FFFF);
            zero_mask_right_pixels = _mm_set_epi32(0x00000000, 0x00000000, 0x00000000, 0x00000000);
            break;
    }

    for (size_t gray_y = 0; gray_y < height; gray_y++) {
        row_offset = gray_y * width;
        for (size_t gray_x = 0; gray_x < width; gray_x += 8) {

            __m128i top_left;
            __m128i top;
            __m128i top_right;
            __m128i bot_left;
            __m128i bot;
            __m128i bot_right;

            // For the first row, the row above is memory not assigned to our application. Set the values to 0 instead.
            if (gray_y > 0) {
                top_left = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*) (image_row_prev + gray_x - 1)),
                                             _mm_setzero_si128());
                top = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*) (image_row_prev + gray_x)),
                                        _mm_setzero_si128());
                top_right = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*) (image_row_prev + gray_x + 1)),
                                              _mm_setzero_si128());
            } else {
                top_left = _mm_setzero_si128();
                top = _mm_setzero_si128();
                top_right = _mm_setzero_si128();
            }
            __m128i left = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*) (image_row_now + gray_x - 1)),
                                            _mm_setzero_si128());
            __m128i right = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*) (image_row_now + gray_x + 1)),
                                     _mm_setzero_si128());

            // For the last row, the row below is memory not assigned to our application. Set the values to 0 instead.
            if (gray_y != height - 1) {
                bot_left = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*) (image_row_next + gray_x - 1)),
                                             _mm_setzero_si128());
                bot = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*) (image_row_next + gray_x)),
                                        _mm_setzero_si128());
                bot_right = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*) (image_row_next + gray_x + 1)),
                                                      _mm_setzero_si128());
            } else {
                bot_left = _mm_setzero_si128();
                bot = _mm_setzero_si128();
                bot_right = _mm_setzero_si128();
            }

            // Set the first left value for the leftmost part of each row to 0 (by definition) to avoid reading
            // incorrect memory.
            if (gray_x == 0) {
                top_left = _mm_and_si128(top_left, zero_mask_drop_first_1_word);
                left = _mm_and_si128(left, zero_mask_drop_first_1_word);
                bot_left = _mm_and_si128(bot_left, zero_mask_drop_first_1_word);
            }

            // Mask the values for the righmost section of each row.
            if (gray_x >= border_pixels_row) {
                top_left = _mm_and_si128(top_left, zero_mask_left_pixels);
                left = _mm_and_si128(left, zero_mask_left_pixels);
                bot_left = _mm_and_si128(bot_left, zero_mask_left_pixels);
                top = _mm_and_si128(top, zero_mask_mid_pixels);
                bot = _mm_and_si128(bot, zero_mask_mid_pixels);
                top_right = _mm_and_si128(top_right, zero_mask_right_pixels);
                right = _mm_and_si128(right, zero_mask_right_pixels);
                bot_right = _mm_and_si128(bot_right, zero_mask_right_pixels);
            }

            // Calculate gradient_vert and gradient_hor in the scheme of Sobel Kernel unroll. Use the absolute values
            // to avoid problems when loading signed values into unsigned values later.
            __m128i gradient_vert = _mm_abs_epi16(_mm_sub_epi16(_mm_add_epi16(top_left,
                                                  _mm_add_epi16(left, _mm_add_epi16(left,
                                                  bot_left))), // positive values
                                                  _mm_add_epi16(top_right,
                                                  _mm_add_epi16(_mm_add_epi16(right, right),
                                                  bot_right)))); // negative values

            __m128i gradient_hor = _mm_abs_epi16(_mm_sub_epi16(_mm_add_epi16(top_left,
                                                 _mm_add_epi16(top, _mm_add_epi16(top,
                                                 top_right))), // positive values
                                                 _mm_add_epi16(bot_left,
                                                 _mm_add_epi16(_mm_add_epi16(bot, bot),
                                                 bot_right)))); // negative values

            // We must use 32 bit values now as the squared values can be larger than 16 bit and we could not detect
            // overflows correctly.
            __m128i gradient_vert_low = _mm_unpacklo_epi16(gradient_vert, _mm_setzero_si128());
            __m128i gradient_vert_high = _mm_unpackhi_epi16(gradient_vert, _mm_setzero_si128());

            __m128i gradient_hor_low = _mm_unpacklo_epi16(gradient_hor, _mm_setzero_si128());
            __m128i gradient_hor_high = _mm_unpackhi_epi16(gradient_hor, _mm_setzero_si128());

            __m128i squared_vert_low = _mm_mullo_epi32(gradient_vert_low, gradient_vert_low);
            __m128i squared_vert_high = _mm_mullo_epi32(gradient_vert_high, gradient_vert_high);

            __m128i squared_hor_low = _mm_mullo_epi32(gradient_hor_low, gradient_hor_low);
            __m128i squared_hor_high = _mm_mullo_epi32(gradient_hor_high, gradient_hor_high);

            __m128i squared_sum_low = _mm_add_epi32(squared_vert_low, squared_hor_low);
            __m128i squared_sum_high = _mm_add_epi32(squared_vert_high, squared_hor_high);

            // LOWER part of squared sum register.
            __m128 squared_sum_float = _mm_cvtepi32_ps(squared_sum_low);
            __m128 gradient_float = _mm_sqrt_ps(squared_sum_float);

            // Clamp to 255 and convert to 8-bit integer.
            __m128i gradient_i = _mm_min_epu32(_mm_cvttps_epi32(gradient_float), max_value_extended);
            __m128i pack_8_low = _mm_packus_epi16(_mm_packus_epi32(gradient_i, gradient_i), _mm_packus_epi32(gradient_i, gradient_i));

            // HIGHER part of squared sum register.
            squared_sum_float = _mm_cvtepi32_ps(squared_sum_high);
            gradient_float = _mm_sqrt_ps(squared_sum_float);

            // Clamp to 255 and convert to 8-bit integer.
            gradient_i = _mm_min_epu32(_mm_cvttps_epi32(gradient_float), max_value_extended);
            __m128i pack_8_high = _mm_packus_epi16(_mm_packus_epi32(gradient_i, gradient_i), _mm_packus_epi32(gradient_i, gradient_i));

            __m128i pack_8 = _mm_alignr_epi8(pack_8_high, pack_8_low, 12);

            // We don't have to treat the last row differently as we allocated more memory than necessary and the write
            // function only checks for width * height.
            _mm_storel_epi64((__m128i*) (result + row_offset + gray_x), pack_8);

        }
        image_row_prev += width;
        image_row_now += width;
        image_row_next += width;
    }
}

void sobel_squareroot_lookup_V1( const uint8_t* img, size_t width, size_t height,
                             float a, float b, float c, void* tmp, uint8_t* result) {

    // Temporary buffer for grayscale image
    uint8_t* grayscale_image = (uint8_t*) tmp;

    // Grayscale conversion
    img_to_grayscale_naive(img, width, height, a, b, c, grayscale_image);
    size_t arr_ct = 0;

    // Sobel edge detection
    int kern_vertical[3][3] = {
            {1, 0, -1},
            {2, 0, -2},
            {1, 0, -1}
    };

    int kern_horizontal[3][3] = {
            {1,  2,  1},
            {0,  0,  0},
            {-1, -2, -1}
    };

    size_t row_indices[height];

    for (size_t i = 0; i < height; i++) {
        row_indices[i] = i * width;
    }

    for (size_t ppm_y = 0; ppm_y < height; ppm_y++) {
        for (size_t ppm_x = 0; ppm_x < width; ppm_x++) {
            int sum_vertical = 0, sum_horizontal = 0;

            // Apply Sobel kernel to pixel
            for (int kern_y = -1; kern_y <= 1; kern_y++) {
                // Only apply kernel values within ppm grid
                if ((ppm_y + kern_y != SIZE_MAX) && (ppm_y + kern_y < height)) {
                    for (int kern_x = -1; kern_x <= 1; kern_x++) {
                        // Only apply kernel values within ppm grid, check for kernel values to be non zero first
                        if (!(kern_x == 0 && kern_y == 0) && (ppm_x + kern_x != SIZE_MAX) &&
                        (ppm_x + kern_x < width)) {
                            uint8_t pixel = grayscale_image[row_indices[ppm_y + kern_y] + (ppm_x + kern_x)];
                            // + 1 to correct the indexing for the kernel matrix
                            sum_vertical += kern_vertical[kern_y + 1][kern_x + 1] * pixel;
                            sum_horizontal += kern_horizontal[kern_y + 1][kern_x + 1] * pixel;
                        } else continue;
                    }
                } else continue;
            }

            // Compute gradient magnitude
            uint8_t magnitude;
            if ((sum_horizontal >= 255 || sum_horizontal <= -255 || sum_vertical >= 255 ||
            sum_vertical <= -255) ||
            ((sum_horizontal >= 181 || sum_horizontal <= -181) &&
            (sum_vertical >= 181 || sum_vertical <= -181))) {
                magnitude = 255;
            } else {
                uint32_t sum_kernel = (sum_vertical * sum_vertical + sum_horizontal * sum_horizontal);
                if (sum_kernel < 65025) {
                    magnitude = squareroot_lookup(sum_kernel);
                } else {
                    magnitude = 255;
                }
            }
            result[arr_ct++] = magnitude;
        }
    }
}

// Separate Convolution
void sobel_V2(const uint8_t* img, size_t width, size_t height,
              float a, float b, float c, void* tmp, uint8_t* result) {

    // Temporary buffer for grayscale image
    uint8_t* grayscale_image = (uint8_t*) tmp;

    // Grayscale conversion
    img_to_grayscale_loop_unroll(img, width, height, a, b, c, grayscale_image);

    size_t image_size = width * height;

    int* temporary_sum = malloc((image_size) * sizeof(int));
    int* temporary_sum_2 = malloc((image_size) * sizeof(int));

    int* temporary_sum_start = temporary_sum;
    int* temporary_sum_2_start = temporary_sum_2;
    uint8_t* result_start = result;

    size_t row_indices[height];
    size_t current_pixel = 0;
    int sum = 0;
    int sum2 = 0;

    for (size_t i = 0; i < height; i++) {
        row_indices[i] = i * width;
    }

    for (size_t gray_y = 0; gray_y < height; gray_y++) {
        for (size_t gray_x = 0; gray_x < width; gray_x++) {
            sum = 0;
            sum2 = 0;
            current_pixel = row_indices[gray_y] + gray_x;
            if (gray_x > 0) {
                sum = grayscale_image[current_pixel - 1];
                sum2 = -grayscale_image[current_pixel - 1];
            }
            sum += grayscale_image[current_pixel] * 2;
            if (gray_x + 1 < width) {
                sum += grayscale_image[current_pixel + 1];
                sum2 += grayscale_image[current_pixel + 1];
            }
            *temporary_sum++ = sum;
            *temporary_sum_2++ = sum2;
        }
    }

    temporary_sum = temporary_sum_start;
    temporary_sum_2 = temporary_sum_2_start;

    // Loop unroll for first row.
    for (size_t gray_x = 0; gray_x < width; gray_x++) {
        sum = 0;
        sum2 = 0;

        sum2 += temporary_sum_2[gray_x] * 2;

        sum += temporary_sum[gray_x + width];
        sum2 += temporary_sum_2[gray_x + width];

        int pixel_sum = sum * sum + sum2 * sum2;
        if (pixel_sum < 65025) {
            *result++ = (uint8_t) sqrt(pixel_sum);
        } else {
            *result++ = 255;
        }
    }

    for (size_t gray_y = 1; gray_y < height - 1; gray_y++) {
        for (size_t gray_x = 0; gray_x < width; gray_x++) {
            sum = 0;
            sum2 = 0;
            current_pixel = row_indices[gray_y] + gray_x;

            sum = -temporary_sum[current_pixel - width];
            sum2 = temporary_sum_2[current_pixel - width];

            sum2 += temporary_sum_2[current_pixel] * 2;

            sum += temporary_sum[current_pixel + width];
            sum2 += temporary_sum_2[current_pixel + width];

            int pixel_sum = sum * sum + sum2 * sum2;
            if (pixel_sum < 65025) {
                *result++ = (uint8_t) sqrt(pixel_sum);
            } else {
                *result++ = 255;
            }
        }
    }

    // Loop unroll for last row.
    for (size_t gray_x = 0; gray_x < width; gray_x++) {
        sum = 0;
        sum2 = 0;
        current_pixel = row_indices[height - 1] + gray_x;

        sum = -temporary_sum[current_pixel - width];
        sum2 = temporary_sum_2[current_pixel - width];

        sum2 += temporary_sum_2[current_pixel] * 2;

        int pixel_sum = sum * sum + sum2 * sum2;
        if (pixel_sum < 65025) {
            *result++ = (uint8_t) sqrt(pixel_sum);
        } else {
            *result++ = 255;
        }
    }

    // Reset the result pointer.
    result = result_start;

    free(temporary_sum);
    free(temporary_sum_2);
}