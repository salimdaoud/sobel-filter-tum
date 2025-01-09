#ifndef PROJEKT_READWRITE_TEST_H
#define PROJEKT_READWRITE_TEST_H

#include "test.h"
#include "../io/readwrite.h"

void test_parse_ppm_header_correct_header(void);

void test_parse_ppm_header_incorrect_header(void);

void test_read_ppm_correct_file(void);

void test_read_ppm_correct_file_parallel(void);

void test_read_ppm_incorrcet_file(void);

void test_write_pgm_file(void);

#endif
