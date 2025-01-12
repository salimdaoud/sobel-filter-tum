#include "sobel.h"

// Sobel operator implementation
void sobel_naive_V0( const uint8_t* img, size_t width, size_t height,
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

void sobel_kernel_unroll_V2( const uint8_t* img, size_t width, size_t height,
                         float a, float b, float c, void* tmp, uint8_t* result) {

    uint8_t* grayscale_image = (uint8_t*) tmp;
    img_to_grayscale_naive(img, width, height, a, b, c, grayscale_image);

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
            gradient_vert = top_left + (left << 1) + bot_left
                            - top_right - (right << 1) - bot_right;

            gradient_hor = top_left + (top << 1) + top_right
                            - bot_left - (bottom << 1) - bot_right;

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

void sobel_SIMD_V3(const uint8_t* img, size_t width, size_t height,
                float a, float b, float c, void* tmp, uint8_t* result) {

    uint8_t* grayscale_image = (uint8_t*)tmp;
    img_to_grayscale_naive(img, width, height, a, b, c, grayscale_image);

    size_t row_offset = 0;

    uint8_t* image_row_prev = grayscale_image - width;
    uint8_t* image_row_now = grayscale_image;
    uint8_t* image_row_next = grayscale_image + width;

    __m128i zero_mask_first_int = _mm_set_epi32(0xFFFFFFFF,0xFFFFFFFF, 0xFFFFFFFF, 0x00000000);
    __m128i zero_mask_last_int = _mm_set_epi32(0x00000000,0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
    __m128i zero_mask_last_two_int = _mm_set_epi32(0x00000000,0x00000000, 0xFFFFFFFF, 0xFFFFFFFF);
    __m128i zero_mask_last_three_int = _mm_set_epi32(0x00000000,0x00000000, 0x00000000, 0xFFFFFFFF);

    for (size_t gray_y = 0; gray_y < (height - 1); gray_y++) {
        row_offset = gray_y * width;
        for (size_t gray_x = 0; gray_x < width; gray_x += 4) {

            __m128i top_row = _mm_setzero_si128();
            __m128i current_row = _mm_setzero_si128();
            __m128i bot_row = _mm_setzero_si128();

            if (gray_y > 0) {
                top_row = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(image_row_prev + gray_x - 1)),
                                            _mm_setzero_si128());
            }
                current_row = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(image_row_now + gray_x - 1)),
                                                _mm_setzero_si128());
                bot_row = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(image_row_next + gray_x - 1)),
                                            _mm_setzero_si128());

            // Process Sobel filter
            __m128i top_left =  _mm_unpacklo_epi16(top_row, _mm_setzero_si128());
            __m128i top =       _mm_unpacklo_epi16(_mm_srli_si128(top_row, 2), _mm_setzero_si128());
            __m128i top_right = _mm_unpacklo_epi16(_mm_srli_si128(top_row, 4), _mm_setzero_si128());
            __m128i left =      _mm_unpacklo_epi16(current_row, _mm_setzero_si128());
            __m128i right =     _mm_unpacklo_epi16(_mm_srli_si128(current_row, 4), _mm_setzero_si128());
            __m128i bot_left =  _mm_unpacklo_epi16(bot_row, _mm_setzero_si128());
            __m128i bot =       _mm_unpacklo_epi16(_mm_srli_si128(bot_row, 2), _mm_setzero_si128());
            __m128i bot_right = _mm_unpacklo_epi16(_mm_srli_si128(bot_row, 4), _mm_setzero_si128());

            if (gray_x == 0) {
                top_left =  _mm_and_si128(top_left, zero_mask_first_int);
                left =      _mm_and_si128(left, zero_mask_first_int);
                bot_left =  _mm_and_si128(bot_left, zero_mask_first_int);
            }

            if (gray_x >= width - 4) {
                if (gray_x == width - 3) {
                    top =           _mm_and_si128(top, zero_mask_last_int);
                    bot =           _mm_and_si128(bot, zero_mask_last_int);
                    top_right =     _mm_and_si128(top_right, zero_mask_last_two_int);
                    right =         _mm_and_si128(right, zero_mask_last_two_int);
                    bot_right =     _mm_and_si128(bot_right, zero_mask_last_two_int);
                }
                if (gray_x == width - 2) {
                    top_left =      _mm_and_si128(top_left, zero_mask_last_int);
                    left =          _mm_and_si128(left, zero_mask_last_int);
                    bot_left =      _mm_and_si128(bot_left, zero_mask_last_int);
                    top =           _mm_and_si128(top, zero_mask_last_two_int);
                    bot =           _mm_and_si128(bot, zero_mask_last_two_int);
                    top_right =     _mm_and_si128(top_right, zero_mask_last_three_int);
                    right =         _mm_and_si128(right, zero_mask_last_three_int);
                    bot_right =     _mm_and_si128(bot_right, zero_mask_last_three_int);;
                }
                if (gray_x == width - 1) {
                    top_left =      _mm_and_si128(top_left, zero_mask_last_two_int);
                    left =          _mm_and_si128(left, zero_mask_last_two_int);
                    bot_left =      _mm_and_si128(bot_left, zero_mask_last_two_int);
                    top =           _mm_and_si128(top, zero_mask_last_three_int);
                    bot =           _mm_and_si128(bot, zero_mask_last_three_int);
                    top_right =     _mm_setzero_si128();
                    right =         _mm_setzero_si128();
                    bot_right =     _mm_setzero_si128();
                }

            }

            // Calculate gradient_vert and gradient_hor
            __m128i gradient_vert = _mm_sub_epi32(_mm_add_epi32(top_left,
                                                  _mm_add_epi32(left, _mm_add_epi32(left,
                                                  bot_left))),
                                                  _mm_add_epi32(top_right,
                                                  _mm_add_epi32(_mm_add_epi32(right, right),
                                                  bot_right)));

            __m128i gradient_hor = _mm_sub_epi32(_mm_add_epi32(top_left,
                                                 _mm_add_epi32(top, _mm_add_epi32(top,
                                                 top_right))),
                                                 _mm_add_epi32(bot_left,
                                                 _mm_add_epi32(_mm_add_epi32(bot, bot),
                                                 bot_right)));

            // Compute squared sum for gradient_vert and gradient_hor
            __m128i squared_vert = _mm_mullo_epi32(gradient_vert, gradient_vert);
            __m128i squared_hor = _mm_mullo_epi32(gradient_hor, gradient_hor);
            __m128i squared_sum = _mm_add_epi32(squared_vert, squared_hor);

            __m128i threshold = _mm_set1_epi32(65025);
            __m128i cmp_result = _mm_cmpgt_epi32(squared_sum, threshold);

            if (__builtin_expect(_mm_test_all_ones(cmp_result), 0)) {
                __m128i max_value = _mm_set1_epi8(255);
                _mm_storel_epi64((__m128i*)(result + row_offset + gray_x), max_value);
            } else {
                // Convert to float and compute the square root
                __m128 squared_sum_float = _mm_cvtepi32_ps(squared_sum);
                __m128 gradient_float = _mm_sqrt_ps(squared_sum_float);

                // Clamp to 255 and convert to 8-bit integer
                __m128i gradient_i = _mm_cvtps_epi32(gradient_float);
                __m128i pack_16 = _mm_packus_epi32(gradient_i, gradient_i);
                __m128i pack_8 = _mm_packus_epi16(pack_16, pack_16);

                _mm_storel_epi64((__m128i*)(result + row_offset + gray_x), pack_8);
            }

        }
        image_row_prev += width;
        image_row_now += width;
        image_row_next += width;
    }

    // loop unroll for last row
    for (size_t gray_x = 0; gray_x < width; gray_x += 4) {

        row_offset = (height - 1) * width;

        __m128i current_0 = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*) (image_row_prev + gray_x - 1)),
                                              _mm_setzero_si128());
        __m128i current_1 = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*) (image_row_now + gray_x - 1)),
                                              _mm_setzero_si128());

        // Process Sobel filter
        __m128i top_left = _mm_unpacklo_epi16(current_0, _mm_setzero_si128());
        __m128i top = _mm_unpacklo_epi16(_mm_srli_si128(current_0, 2), _mm_setzero_si128());
        __m128i top_right = _mm_unpacklo_epi16(_mm_srli_si128(current_0, 4), _mm_setzero_si128());
        __m128i left = _mm_unpacklo_epi16(current_1, _mm_setzero_si128());
        __m128i right = _mm_unpacklo_epi16(_mm_srli_si128(current_1, 4), _mm_setzero_si128());

        if (gray_x == 0) {
            top_left = _mm_and_si128(top_left, zero_mask_first_int);
            left = _mm_and_si128(left, zero_mask_first_int);
        }

        if (gray_x >= width - 4) {
            if (gray_x == width - 3) {
                top = _mm_and_si128(top, zero_mask_last_int);
                top_right = _mm_and_si128(top_right, zero_mask_last_two_int);
                right = _mm_and_si128(right, zero_mask_last_two_int);
            }
            if (gray_x == width - 2) {
                top_left = _mm_and_si128(top_left, zero_mask_last_int);
                left = _mm_and_si128(left, zero_mask_last_int);
                top = _mm_and_si128(top, zero_mask_last_two_int);
                top_right = _mm_and_si128(top_right, zero_mask_last_three_int);
                right = _mm_and_si128(right, zero_mask_last_three_int);
            }
            if (gray_x == width - 1) {
                top_left = _mm_and_si128(top_left, zero_mask_last_two_int);
                left = _mm_and_si128(left, zero_mask_last_two_int);
                top = _mm_and_si128(top, zero_mask_last_three_int);
                top_right = _mm_setzero_si128();
                right = _mm_setzero_si128();
            }

        }

        // Calculate gradient_vert and gradient_hor
        __m128i gradient_vert = _mm_sub_epi32(_mm_add_epi32(top_left,
                                              _mm_add_epi32(left, left)),
                                              _mm_add_epi32(top_right,
                                              _mm_add_epi32(right, right)));

        __m128i gradient_hor = _mm_add_epi32(top_left,
                                             _mm_add_epi32(top, _mm_add_epi32(top,
                                             top_right)));

        // Compute squared sum for gradient_vert and gradient_hor
        __m128i squared_vert = _mm_mullo_epi32(gradient_vert, gradient_vert);
        __m128i squared_hor = _mm_mullo_epi32(gradient_hor, gradient_hor);
        __m128i squared_sum = _mm_add_epi32(squared_vert, squared_hor);
        __m128 squared_sum_float = _mm_cvtepi32_ps(squared_sum);
        __m128 gradient_float = _mm_sqrt_ps(squared_sum_float);

        // Clamp to 255 and convert to 8-bit integer
        __m128i gradient_i = _mm_cvtps_epi32(gradient_float);
        __m128i pack_16 = _mm_packus_epi32(gradient_i, gradient_i);
        __m128i pack_8 = _mm_packus_epi16(pack_16, pack_16);

        if (gray_x > width - 4) {
                if (gray_x == width - 3) {
                    // Write only the first 3 values
                    int32_t temp[4];
                    _mm_storeu_si128((__m128i*) temp, pack_8);
                    result[row_offset + gray_x] = (uint8_t) temp[0];
                    result[row_offset + gray_x + 1] = (uint8_t) temp[1];
                    result[row_offset + gray_x + 2] = (uint8_t) temp[2];
                }
                if (gray_x == width - 2) {
                    int32_t temp[4];
                    _mm_storeu_si128((__m128i*) temp, pack_8);
                    result[row_offset + gray_x] = (uint8_t) temp[0];
                    result[row_offset + gray_x + 1] = (uint8_t) temp[1];
                }
                if (gray_x == width - 1) {
                    // Write only the first value
                    int32_t temp[4];
                    _mm_storeu_si128((__m128i*) temp, pack_8);
                    result[row_offset + gray_x] = (uint8_t) temp[0];
                }
        } else {
            // Store the result
            _mm_storel_epi64((__m128i*) (result + row_offset + gray_x), pack_8);
        }
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

void sobel_separated_convolution_V4( const uint8_t* img, size_t width, size_t height,
                                 float a, float b, float c, void* tmp, uint8_t* result) {

    // Temporary buffer for grayscale image
    uint8_t* grayscale_image = (uint8_t*) tmp;

    // Grayscale conversion
    img_to_grayscale_naive(img, width, height, a, b, c, grayscale_image);

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
            sum += (grayscale_image[current_pixel] << 1);
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


        //loop unroll for first row
    for (size_t gray_x = 0; gray_x < width; gray_x++) {
        sum = 0;
        sum2 = 0;

        sum2 += (temporary_sum_2[gray_x] << 1);

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

            sum2 += (temporary_sum_2[current_pixel] << 1);

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

        // loop unroll for last row
    for (size_t gray_x = 0; gray_x < width; gray_x++) {
        sum = 0;
        sum2 = 0;
        current_pixel = row_indices[height - 1] + gray_x;

        sum = -temporary_sum[current_pixel - width];
        sum2 = temporary_sum_2[current_pixel - width];

        sum2 += (temporary_sum_2[current_pixel] << 1);

        int pixel_sum = sum * sum + sum2 * sum2;
        if (pixel_sum < 65025) {
            *result++ = (uint8_t) sqrt(pixel_sum);
        } else {
            *result++ = 255;
        }
    }

    result = result_start;

    free(temporary_sum);
    free(temporary_sum_2);
}