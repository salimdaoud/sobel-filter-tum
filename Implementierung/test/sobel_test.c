#include "sobel_test.h"

void test_sobel_naive(void) {
    uint8_t img [27] = {177, 233, 117, 244, 139, 96, 151, 94, 147,
                        125, 118, 245, 95, 78, 207, 163, 181, 82,
                        20, 196, 64, 198, 178, 31, 122, 253, 236};

    uint8_t expected [9] = {255, 255, 255,
                            255, 61, 255,
                            255, 255, 255};

    uint8_t gray[9] = {0};
    uint8_t actual[9] = {0};

    sobel_naive(img, 3, 3, 0.299f, 0.587f, 0.114f, gray, actual, 0);

    /*for (size_t i = 0; i < 9; i++){
        printf("%d\n", actual[i]);
    }*/

    assert_uint8_array_equal(expected, actual, 9);
}

void test_sobel_kernel_unroll(void){
    uint8_t img [27] = {0, 8, 17, 19, 23, 26, 32, 33, 41,
                        50, 60, 72, 84, 91, 10, 20, 35, 37,
                        45, 40, 53, 72, 99, 20, 21, 23, 44};

    uint8_t expected [9] = {230, 247, 185,
                            255, 152, 255,
                            255, 254, 255};

    uint8_t gray[9] = {0};
    uint8_t actual[9] = {0};

    sobel_kernel_unroll(img, 3, 3, 0.299f, 0.587f, 0.114f, gray, actual, 0);

    for (size_t i = 0; i < 9; i++){
        printf("%d\n", actual[i]);
    }

    assert_uint8_array_equal(expected, actual, 9);
}

void test_sobel_SIMD(void) {
    uint8_t img [27] = {0, 8, 17, 19, 23, 26, 32, 33, 41,
                        50, 60, 72, 84, 91, 10, 20, 35, 37,
                        45, 40, 53, 72, 99, 20, 21, 23, 44};

    uint8_t expected [9] = {230, 247, 185,
                            255, 152, 255,
                            255, 254, 255};

    uint8_t gray[9] = {0};
    uint8_t actual[9] = {0};

    sobel_SIMD(img, 3, 3, 0.299f, 0.587f, 0.114f, gray, actual, 0);

    for (size_t i = 0; i < 9; i++){
        printf("%d\n", actual[i]);
    }

    assert_uint8_array_equal(expected, actual, 9);
}

void test_sobel_squareroot_lookup(void) {
    uint8_t img [27] = {0, 8, 17, 19, 23, 26, 32, 33, 41,
                        50, 60, 72, 84, 91, 10, 20, 35, 37,
                        45, 40, 53, 72, 99, 20, 21, 23, 44};

    uint8_t expected [9] = {230, 247, 185,
                            255, 152, 255,
                            255, 254, 255};

    uint8_t gray[9] = {0};
    uint8_t actual[9] = {0};

    sobel_squareroot_lookup(img, 3, 3, 0.299f, 0.587f, 0.114f, gray, actual, 0);

    /*for (size_t i = 0; i < 9; i++){
        printf("%d\n", actual[i]);
    }*/

    assert_uint8_array_equal(expected, actual, 9);

}

void test_sobel_separated_convolution(void) {
    uint8_t img [27] = {0, 8, 17, 19, 23, 26, 32, 33, 41,
                        50, 60, 72, 84, 91, 10, 20, 35, 37,
                        45, 40, 53, 72, 99, 20, 21, 23, 44};

    uint8_t expected [9] = {230, 247, 185,
                            255, 152, 255,
                            255, 254, 255};

    uint8_t gray[9] = {0};
    uint8_t actual[9] = {0};

    sobel_separated_convolution (img, 3, 3, 0.299f, 0.587f, 0.114f, gray, actual, 0);

    /*for (size_t i = 0; i < 9; i++){
        printf("%d\n", actual[i]);
    }*/

    assert_uint8_array_equal(expected, actual, 9);

}
