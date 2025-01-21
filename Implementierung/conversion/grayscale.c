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

    size_t rgb_values_count = width * height * 3;

    for (size_t i = 0, j = 0; i < rgb_values_count; i += 3, j++) {
        gray[j] = (uint8_t)((img[i] >> 2) + (img[i + 1] >> 1) + (img[i + 2] >> 2));
    }
}

// Source: https://stackoverflow.com/questions/57832444/efficient-c-code-no-libs-for-image-transformation-into-custom-rgb-pixel-grey/57844027#57844027

//Convert from RGBRGBRGB... to RRR..., GGG..., BBB...
//Input: Two XMM registers (16 + 8 = 24 uint8 elements) ordered RGBRGB...
//Output: Three XMM registers ordered RRR..., GGG... and BBB...
//        Unpack the result from uint8 elements to uint16 elements.
static __inline void GatherRGBx8(const __m128i r_5_bgr_4_bgr_3_bgr_2_bgr_1_bgr_0,
                                 const __m128i bgr_7_bgr_6_bg_5,
                                    __m128i r_76543210,
                                    __m128i g_76543210,
                                    __m128i b_76543210)
{
//Shuffle mask for gathering 4 R elements, 4 G elements and 4 B elements (also set last 4 elements to duplication of first 4 elements).
const __m128i shuffle_mask = _mm_set_epi8(9,6,3,0, 11,8,5,2, 10,7,4,1, 9,6,3,0);

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
r_76543210 = _mm_cvtepu8_epi16(b_7654_g_7654_r_7654_r_3210);
g_76543210 = _mm_cvtepu8_epi16(r_7654_b_7654_g_7654_g_3210);
b_76543210 = _mm_cvtepu8_epi16(z_0000_r_7654_b_7654_b_3210);
}



// Calculate 8 Grayscale elements from 8 RGB elements.
// Y = 0.2989*R + 0.5870*G + 0.1140*B
// Conversion model used by MATLAB https://www.mathworks.com/help/matlab/ref/rgb2gray.html
static __inline __m128i Rgb2Yx8(__m128i r7_r6_r5_r4_r3_r2_r1_r0,
                                __m128i g7_g6_g5_g4_g3_g2_g1_g0,
                                __m128i b7_b6_b5_b4_b3_b2_b1_b0)
{
    // Each coefficient is expanded by 2^15, and rounded to int16 (add 0.5 for rounding).
    const __m128i r_coef = _mm_set1_epi16((short)(0.2989*32768.0 + 0.5));  //8 coefficients - R scale factor.
    const __m128i g_coef = _mm_set1_epi16((short)(0.5870*32768.0 + 0.5));  //8 coefficients - G scale factor.
    const __m128i b_coef = _mm_set1_epi16((short)(0.1140*32768.0 + 0.5));  //8 coefficients - B scale factor.

    // Multiply input elements by 64 for improved accuracy.
    r7_r6_r5_r4_r3_r2_r1_r0 = _mm_slli_epi16(r7_r6_r5_r4_r3_r2_r1_r0, 6);
    g7_g6_g5_g4_g3_g2_g1_g0 = _mm_slli_epi16(g7_g6_g5_g4_g3_g2_g1_g0, 6);
    b7_b6_b5_b4_b3_b2_b1_b0 = _mm_slli_epi16(b7_b6_b5_b4_b3_b2_b1_b0, 6);

    // Use the special intrinsic _mm_mulhrs_epi16 that calculates round(r*r_coef/2^15).
    // Calculate Y = 0.2989*R + 0.5870*G + 0.1140*B (use fixed point computations)
    __m128i y7_y6_y5_y4_y3_y2_y1_y0 = _mm_add_epi16(_mm_add_epi16(
                                                            _mm_mulhrs_epi16(r7_r6_r5_r4_r3_r2_r1_r0, r_coef),
                                                            _mm_mulhrs_epi16(g7_g6_g5_g4_g3_g2_g1_g0, g_coef)),
                                                    _mm_mulhrs_epi16(b7_b6_b5_b4_b3_b2_b1_b0, b_coef));

    // Divide result by 64.
    y7_y6_y5_y4_y3_y2_y1_y0 = _mm_srli_epi16(y7_y6_y5_y4_y3_y2_y1_y0, 6);

    return y7_y6_y5_y4_y3_y2_y1_y0;
}




// Convert single row from RGB to Grayscale (use SSE intrinsics).
// I0 points source row, and J0 points destination row.
// I0 -> rgbrgbrgbrgbrgbrgb...
// J0 -> yyyyyy
static void Rgb2GraySingleRow_useSSE(const unsigned char I0[],
                                     const int image_width,
                                     unsigned char J0[])
{
    int x;      // Index in J0.
    int srcx;   // Index in I0.
    __m128i r7_r6_r5_r4_r3_r2_r1_r0;
    __m128i g7_g6_g5_g4_g3_g2_g1_g0;
    __m128i b7_b6_b5_b4_b3_b2_b1_b0;

    srcx = 0;

    // Process 8 pixels per iteration.
    for (x = 0; x < image_width; x += 8)
    {
        // Load 8 elements of each color channel R,G,B from first row.
        __m128i r5_b4_g4_r4_b3_g3_r3_b2_g2_r2_b1_g1_r1_b0_g0_r0 = _mm_loadu_si128((__m128i*)&I0[srcx]);     //Unaligned load of 16 uint8 elements
        __m128i b7_g7_r7_b6_g6_r6_b5_g5                         = _mm_loadu_si128((__m128i*)&I0[srcx+16]);  //Unaligned load of (only) 8 uint8 elements (lower half of XMM register).

        // Separate RGB, and put together R elements, G elements and B elements (together in same XMM register).
        // Result is also unpacked from uint8 to uint16 elements.
        GatherRGBx8(r5_b4_g4_r4_b3_g3_r3_b2_g2_r2_b1_g1_r1_b0_g0_r0,
                    b7_g7_r7_b6_g6_r6_b5_g5,
                    r7_r6_r5_r4_r3_r2_r1_r0,
                    g7_g6_g5_g4_g3_g2_g1_g0,
                    b7_b6_b5_b4_b3_b2_b1_b0);


        // Calculate 8 Y elements.
        __m128i y7_y6_y5_y4_y3_y2_y1_y0 = Rgb2Yx8(r7_r6_r5_r4_r3_r2_r1_r0,
                                                  g7_g6_g5_g4_g3_g2_g1_g0,
                                                  b7_b6_b5_b4_b3_b2_b1_b0);

        // Pack uint16 elements to 16 uint8 elements (put result in single XMM register). Only lower 8 uint8 elements are relevant.
        __m128i j7_j6_j5_j4_j3_j2_j1_j0 = _mm_packus_epi16(y7_y6_y5_y4_y3_y2_y1_y0, y7_y6_y5_y4_y3_y2_y1_y0);

        // Store 8 elements of Y in row Y0, and 8 elements of Y in row Y1.
        _mm_storel_epi64((__m128i*)&J0[x], j7_j6_j5_j4_j3_j2_j1_j0);

        srcx += 24; //Advance 24 source bytes per iteration.
    }
}





//Convert image I from pixel ordered RGB to Grayscale format.
//Conversion formula: Y = 0.2989*R + 0.5870*G + 0.1140*B (Rec.ITU-R BT.601)
//Formula is based on MATLAB rgb2gray function: https://www.mathworks.com/help/matlab/ref/rgb2gray.html
//Implementation uses SSE intrinsics for performance optimization.
//Use fixed point computations for better performance.
//I - Input image in pixel ordered RGB format.
//image_width - Number of columns of I.
//image_height - Number of rows of I.
//J - Destination "image" in Grayscale format.

//I is pixel ordered RGB color format (size in bytes is image_width*image_height*3):
//RGBRGBRGBRGBRGBRGB
//RGBRGBRGBRGBRGBRGB
//RGBRGBRGBRGBRGBRGB
//RGBRGBRGBRGBRGBRGB
//
//J is in Grayscale format (size in bytes is image_width*image_height):
//YYYYYY
//YYYYYY
//YYYYYY
//YYYYYY
//
//Limitations:
//1. image_width must be a multiple of 8.
//2. I and J must be two separate arrays (in place computation is not supported).
//3. Rows of I and J are continues in memory (bytes stride is not supported, [but simple to add]).
//
//Comments:
//1. The conversion formula is incorrect, but it's a commonly used approximation.
//2. Code uses SSE 4.1 instruction set.
//   Better performance can be archived using AVX2 implementation.
//   (AVX2 is supported by Intel Core 4'th generation and above, and new AMD processors).
//3. The code is not the best SSE optimization:
//   Uses unaligned load and store operations.
//   Utilize only half XMM register in few cases.
//   Instruction selection is probably sub-optimal.
void Rgb2Gray_useSSE(const unsigned char I[],
                     const int image_width,
                     const int image_height,
                     unsigned char J[])
{
    //I0 points source image row.
    const unsigned char *I0;  //I0 -> rgbrgbrgbrgbrgbrgb...

    //J0 points destination image row.
    unsigned char *J0;  //J0 -> YYYYYY

    int y;  //Row index

    //Process one row per iteration.
    for (y = 0; y < image_height; y ++)
    {
        I0 = &I[y*image_width*3];       //Input row width is image_width*3 bytes (each pixel is R,G,B).

        J0 = &J[y*image_width];         //Output Y row width is image_width bytes (one Y element per pixel).

        //Convert row I0 from RGB to Grayscale.
        Rgb2GraySingleRow_useSSE(I0,
                                 image_width,
                                 J0);
    }

}