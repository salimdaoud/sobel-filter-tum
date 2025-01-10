#include "readwrite_test.h"

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
    char file_data[] =  "test/binary_correct_file.ppm";
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
    char file_data[] =  "test/binary_correct_file.ppm";
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
    char file_data[] =  "test/binary_incorrect_file.ppm";
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

//max_val is is less than 0 or greater 255 
void test_read_ppm_incorrcet_file_maxval(void) {
    char file_data[] =  "test/binary_incorrect_file_maxval.ppm";
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
    char file_name[] = "test/binary_write_file.pgm";
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
