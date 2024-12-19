#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

void sobel(
    const uint8_t* img, size_t width, size_t height,
    float a, float b, float c,
    void* tmp,
    uint8_t* result
);

void readPPM(const char* filename, int* width, int* height, uint8_t** rgbData);

void writePGM(const char* filename, const uint8_t* data, int width, int height);
