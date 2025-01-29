#include "sobel_test.h"

void test_sobel_naive_V0(void) {
    uint8_t img [27] = {177, 233, 117, 244, 139, 96, 151, 94, 147,
                        125, 118, 245, 95, 78, 207, 163, 181, 82,
                        20, 196, 64, 198, 178, 31, 122, 253, 236};

    uint8_t expected [9] = {255, 255, 255,
                            255, 61, 255,
                            255, 255, 255};

    uint8_t gray[9] = {0};
    uint8_t actual[9] = {0};

    sobel(img, 3, 3, 0.299f, 0.587f, 0.114f, gray, actual);

    bool test_result = uint8_array_equal(expected, actual, 9);
    ASSERT_TRUE("Sobel Naive Basic Test", test_result);
    if (!test_result) {
        print_matrix_test_details(expected, actual, 9);
    }
}

void test_sobel_kernel_unroll_V2(void){
    uint8_t img [27] = {0, 8, 17, 19, 23, 26, 32, 33, 41,
                        50, 60, 72, 84, 91, 10, 20, 35, 37,
                        45, 40, 53, 72, 99, 20, 21, 23, 44};

    uint8_t expected [9] = {230, 247, 185,
                            255, 152, 255,
                            255, 254, 255};

    uint8_t gray[9] = {0};
    uint8_t actual[9] = {0};

    sobel_V1(img, 3, 3, 0.299f, 0.587f, 0.114f, gray, actual);



    bool test_result = uint8_array_equal(expected, actual, 9);
    ASSERT_TRUE("Sobel V2 Basic Test", test_result);
    if (!test_result) {
        print_matrix_test_details(expected, actual, 9);
    }
}

void test_sobel_SIMD_V3(void) {
    uint8_t img [36] = {0, 8, 17, 19, 23, 26, 32, 33, 41,50, 60, 72,
                        84, 91, 10, 20, 35, 37,45, 40, 53,72, 99, 20,
                        21, 23, 44,158, 56, 218, 158,56,218, 158,56,218};

    uint8_t expected [12] = {202, 181, 230,230,
                             220, 255,255, 255,
                             255, 218,201,255};

    uint8_t gray[12] = {0};

    uint8_t* actual = malloc(12);

    sobel_V3(img, 4, 3, 0.299f, 0.587f, 0.114f, gray, actual);

    bool test_result = uint8_array_equal(expected, actual, 12);
    ASSERT_TRUE("Sobel V3 Basic Test", test_result);
    if (!test_result) {
        print_matrix_test_details(expected, actual, 12);
    }
}

void test_sobel_squareroot_lookup_V1(void) {
    uint8_t img [27] = {0, 8, 17, 19, 23, 26, 32, 33, 41,
                        50, 60, 72, 84, 91, 10, 20, 35, 37,
                        45, 40, 53, 72, 99, 20, 21, 23, 44};

    uint8_t expected [9] = {230, 247, 185,
                            255, 152, 255,
                            255, 254, 255};

    uint8_t gray[9] = {0};
    uint8_t actual[9] = {0};

    sobel_squareroot_lookup_V1(img, 3, 3, 0.299f, 0.587f, 0.114f, gray, actual);

    bool test_result = uint8_array_equal(expected, actual, 9);
    ASSERT_TRUE("Sobel V1 Basic Test", test_result);
    if (!test_result) {
        print_matrix_test_details(expected, actual, 9);
    }

}

void test_sobel_separated_convolution_V4(void) {
    uint8_t img [27] = {0, 8, 17, 19, 23, 26, 32, 33, 41,
                        50, 60, 72, 84, 91, 10, 20, 35, 37,
                        45, 40, 53, 72, 99, 20, 21, 23, 44};

    uint8_t expected [9] = {230, 247, 185,
                            255, 152, 255,
                            255, 254, 255};

    uint8_t gray[9] = {0};
    uint8_t actual[9] = {0};

    sobel_V2(img, 3, 3, 0.299f, 0.587f, 0.114f, gray, actual);

    /*for (size_t i = 0; i < 9; i++){
        printf("%d\n", actual[i]);
    }*/

    bool test_result = uint8_array_equal(expected, actual, 9);
    ASSERT_TRUE("Sobel V4 Basic Test", test_result);
    if (!test_result) {
        print_matrix_test_details(expected, actual, 9);
    }

}
