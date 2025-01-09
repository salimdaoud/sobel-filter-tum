#include "test.h"


void assert_uint8_array_equal(const uint8_t* expected, const uint8_t* actual, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        if (expected[i] != actual[i]) {
            printf("FAIL\n");
            return;
        }
    }
    printf("SUCCESS\n");
}

void assert_int_equal(int expected, int actual) {
        if (expected != actual) {
            printf("FAIL\n");
            return;
        }
    printf("SUCCESS\n");
}


void test_img_to_grayscale_naive(void) {
    uint8_t img [30] = {177, 233, 117, 244, 139, 96, 151, 94, 147, 125,
                        118, 245, 95, 78, 207, 163, 181, 82, 20, 196,
                        64, 198, 178, 31, 122, 253, 236, 153, 47, 215};
                    
    uint8_t expected [10] = {203, 165, 117, 134, 97, 164, 128, 167, 211, 97};

    uint8_t actual[10] = {0}; 

    img_to_grayscale_naive(img, 10, 1, 0.299f, 0.587f, 0.114f, actual, 0);
    /*for (size_t i = 0; i < 10; ++i){
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

    img_to_grayscale_naive(img, 10, 1, 1.0E-3, 1.0E-5, 1.0E-7, actual, 0);
    /*for (size_t i = 0; i < 10; ++i){
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

    img_to_grayscale_SIMD(img, 10, 1, 0.299f, 0.587f, 0.114f, actual, 0);
        /*for (size_t i = 0; i < 10; ++i){
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

    img_to_grayscale(img, 10, 1, 0.299f, 0.587f, 0.114f, actual, 0);
        /*for (size_t i = 0; i < 10; ++i){
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

    img_to_grayscale_bitshift(img, 10, 1, 0.299f, 0.587f, 0.114f, actual, 0);
        /*for (size_t i = 0; i < 10; ++i){
        printf("%d\n", actual[i]);
        }*/

    assert_uint8_array_equal(expected, actual, 10);  

}

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

    /*for (size_t i = 0; i < 9; ++i){
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

    for (size_t i = 0; i < 9; ++i){
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

    for (size_t i = 0; i < 9; ++i){
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

    /*for (size_t i = 0; i < 9; ++i){
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

    /*for (size_t i = 0; i < 9; ++i){
        printf("%d\n", actual[i]);
    }*/

    assert_uint8_array_equal(expected, actual, 9);

}

void test_parse_ppm_header_correct_header(void) {
    char file_data[] =  "P6\n3 3\n255\n";
    int width, height, max_val;
    parse_ppm_header(file_data, &width, &height, &max_val);
    assert_int_equal(3, width);
    assert_int_equal(3, height);
    assert_int_equal(255, max_val);
}

void test_parse_ppm_header_incorrect_header(void) {
    char* file_data =  "P3\n3 3\n255\n";
    int width, height, max_val;
    size_t header_size = parse_ppm_header(file_data, &width, &height, &max_val);
    assert_int_equal(-1, header_size);
}

void test_read_ppm_correct_file(void) {
    char file_data[] =  "testing/binary_correct_file.ppm";
    int width, height;
    uint8_t* pixel_rgb_data;
    read_ppm_file(file_data, &width, &height, &pixel_rgb_data, 0);
    uint8_t expected_pixels [] = {10, 20, 30, 50, 60, 70, 80, 90, 100,
                                110, 5, 15, 25, 40, 59, 70, 4, 2,
                                11, 12, 76, 80, 95, 48, 35, 74, 55,
                                };
    assert_uint8_array_equal(expected_pixels, pixel_rgb_data, 27);
    /*for (int i=0; i < 27; i++){
        printf("%d\n", pixel_rgb_data[i]);
    }*/
}

void test_read_ppm_correct_file_parallel(void) {
    char file_data[] =  "testing/binary_correct_file.ppm";
    int width, height;
    uint8_t* pixel_rgb_data;
    read_ppm_file(file_data, &width, &height, &pixel_rgb_data, 1);
    uint8_t expected_pixels [] = {10, 20, 30, 50, 60, 70, 80, 90, 100,
                                110, 5, 15, 25, 40, 59, 70, 4, 2,
                                11, 12, 76, 80, 95, 48, 35, 74, 55,
                                };
    assert_uint8_array_equal(expected_pixels, pixel_rgb_data, 27);
    /*for (int i=0; i < 27; i++){
        printf("%d\n", pixel_rgb_data[i]);
    }*/
}

//width and height do not correspond to given pixels
void test_read_ppm_incorrcet_file(void) {
    char file_data[] =  "testing/binary_incorrect_file.ppm";
    int width, height;
    uint8_t* pixel_rgb_data;
    read_ppm_file(file_data, &width, &height, &pixel_rgb_data, 0);
    uint8_t expected_pixels [] = {10, 20, 30, 50, 60, 70, 80, 90, 100,
                                110, 5, 15, 25, 40, 59, 70, 4, 2,
                                11, 12, 76, 80, 95, 48, 35, 74, 55,
                                };
    assert_uint8_array_equal(expected_pixels, pixel_rgb_data, 27);
    /*for (int i=0; i < 27; i++){
        printf("%d\n", pixel_rgb_data[i]);
    }*/
}

void test_write_pgm_file(void) {
    char file_name[] = "testing/binary_write_file.pgm";
    int width = 3;
    int height = 3;
    uint8_t sobel_pixels[] = {
        10, 20, 30,
        40, 50, 60,
        70, 80, 90
    };

    // Call the function to write the file
    write_pgm_file(file_name, sobel_pixels, width, height, 1);

    // Open the file for reading
    FILE *file = fopen(file_name, "rb");
    assert(file != NULL);

    // Read the file contents into a buffer
    uint8_t file_data[128];
    fread(file_data, sizeof(uint8_t), sizeof(file_data), file);
    fclose(file);

    // Construct the expected file data
    char expected_header[128];
    int header_size = snprintf(expected_header, sizeof(expected_header), "P5\n%d %d\n255\n", width, height);

    // Validate header
    assert(memcmp(file_data, expected_header, header_size) == 0);

    // Validate pixel data
    for (int i = 0; i < 9; i++) {
        assert(file_data[header_size + i] == sobel_pixels[i]);
    }

    printf("Test passed!\n");
}









