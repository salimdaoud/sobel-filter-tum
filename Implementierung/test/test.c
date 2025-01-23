#include "test.h"

const char *test_name = "";
const char *assertion = "";
const char *file = "";
const char* function_name = "";
int global_total_tests = 0;
int global_failed_tests = 0;

bool uint8_array_equal(const uint8_t* expected, const uint8_t* actual, size_t size) {
    for (size_t i = 0; i < size; i++) {
        if (expected[i] != actual[i]) {
            return false;
        }
    }
    return true;
}


bool int_equal(int expected, int actual) {
    if (expected != actual) {
        return false;
    }
    return true;
}

void print_matrix_test_details(uint8_t* expected, uint8_t* actual, size_t size) {
    printf("%-12s %-25s\n", "", "Details:");
    printf("%-20s %-3s %-12s %5s\n","", "", "Expected:", "Got:");
    for (size_t i = 0; i < 9; i++){
        printf("%-20s [%zu]%-4s %5d %8d\n","", i, "", expected[i], actual[i]);
    }
    printf("\n");
}









