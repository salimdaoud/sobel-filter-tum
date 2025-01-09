#include "test.h"

void assert_uint8_array_equal(const uint8_t* expected, const uint8_t* actual, size_t size) {
    for (size_t i = 0; i < size; i++) {
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









