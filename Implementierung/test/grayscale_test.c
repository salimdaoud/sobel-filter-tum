#include "grayscale_test.h"

void test_img_to_grayscale_naive(void) {
    uint8_t img [30] = {177, 233, 117, 244, 139, 96, 151, 94, 147, 125,
                        118, 245, 95, 78, 207, 163, 181, 82, 20, 196,
                        64, 198, 178, 31, 122, 253, 236, 153, 47, 215};

    uint8_t expected [10] = {203, 165, 117, 134, 97, 164, 128, 167, 211, 97};

    uint8_t actual[10] = {0};

    img_to_grayscale_naive(img, 10, 1, 0.299f, 0.587f, 0.114f, actual);
    /*for (size_t i = 0; i < 10; i++{
        printf("%d\n", actual[i]);
    }*/

    assert_uint8_array_equal(expected, actual, 10);
}

void test_img_to_grayscale_naive_little_weights(void) {
    uint8_t img [30] = {177, 233, 117, 244, 139, 96, 151, 94, 147, 125,
                        118, 245, 95, 78, 207, 163, 181, 82, 20, 196,
                        64, 198, 178, 31, 122, 253, 236, 153, 47, 215};

    uint8_t expected [10] = {177, 242, 150, 124, 94, 163, 21, 197, 123, 151};

    uint8_t actual[10] = {0};

    img_to_grayscale_naive(img, 10, 1, 1.0E-3, 1.0E-5, 1.0E-7, actual);
    /*for (size_t i = 0; i < 10; i++){
        printf("%d\n", actual[i]);
    }*/

    assert_uint8_array_equal(expected, actual, 10);
}

void test_img_to_grayscale_SIMD(void) {
    uint8_t img [30] = {177, 233, 117, 244, 139, 96, 151, 94, 147, 125,
                        118, 245, 95, 78, 207, 163, 181, 82, 20, 196,
                        64, 198, 178, 31, 122, 253, 236, 153, 47, 215};

    uint8_t expected [10] = {203, 165, 117, 134, 97, 164, 128, 167, 211, 97};

    uint8_t actual[10] = {0};

    img_to_grayscale_SIMD(img, 10, 1, 0.299f, 0.587f, 0.114f, actual);
    /*for (size_t i = 0; i < 10; i++){
    printf("%d\n", actual[i]);
    }*/

    assert_uint8_array_equal(expected, actual, 10);

}

void test_img_to_grayscale(void) {
    uint8_t img [30] = {177, 233, 117, 244, 139, 96, 151, 94, 147, 125,
                        118, 245, 95, 78, 207, 163, 181, 82, 20, 196,
                        64, 198, 178, 31, 122, 253, 236, 153, 47, 215};

    uint8_t expected [10] = {203, 165, 117, 134, 97, 164, 128, 167, 211, 97};

    uint8_t actual[10] = {0};

    img_to_grayscale(img, 10, 1, 0.299f, 0.587f, 0.114f, actual);
    /*for (size_t i = 0; i < 10; i++){
    printf("%d\n", actual[i]);
    }*/

    assert_uint8_array_equal(expected, actual, 10);

}

void test_img_to_grayscale_bitshift(void) {
    uint8_t img [30] = {177, 233, 117, 244, 139, 96, 151, 94, 147, 125,
                        118, 245, 95, 78, 207, 163, 181, 82, 20, 196,
                        64, 198, 178, 31, 122, 253, 236, 153, 47, 215};

    uint8_t expected [10] = {203, 165, 117, 134, 97, 164, 128, 167, 211, 97};

    uint8_t actual[10] = {0};

    img_to_grayscale_bitshift(img, 10, 1, 0.299f, 0.587f, 0.114f, actual);
    /*for (size_t i = 0; i < 10; i++){
    printf("%d\n", actual[i]);
    }*/

    assert_uint8_array_equal(expected, actual, 10);

}
