#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../conversion/grayscale.h"
#include "../conversion/sobel.h"

// loosely based on: https://www.reddit.com/r/C_Programming/comments/151bc38/comment/js7oxgt/?utm_source=share&utm
// _medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button

extern const char* function_name;
extern const char* file;
extern int global_total_tests;
extern int global_failed_tests;

#define ASSERT_TRUE(test_name, expression)\
          do {\
            file = __FILE__;\
            function_name = __FUNCTION__;\
            if(expression) {\
                printf("\033[32m%-12s\033[0m %-25s\n"\
                "============================================================================================"\
                "======================\n",\
                "SUCCESSFUL:",  test_name);\
                global_total_tests++;\
            } else {\
                printf("\033[31m%-12s\033[0m %-25s\n%-12s %-25s \033[1;34m%s()\033[0m in %s\n",\
                "FAILED:", test_name,"", "Result does not match expectations, see", function_name, file);\
                global_total_tests++;\
                global_failed_tests++;\
            }\
          } while(0)

bool uint8_array_equal(const uint8_t* expected, const uint8_t* actual, size_t size);

bool int_equal(int expected, int actual);

void print_matrix_test_details(uint8_t* expected, uint8_t* actual, size_t size);







