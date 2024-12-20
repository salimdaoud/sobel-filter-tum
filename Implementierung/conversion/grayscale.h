#ifndef GRAYSCALE_H
#define GRAYSCALE_H

#include <stdint.h>
#include <stdlib.h>

void grayscale(const uint8_t* img, size_t width, size_t height,
               float a, float b, float c, uint8_t* tmp);

#endif