#include "grayscale.h"

void grayscale(const uint8_t* img, size_t width, size_t height,
    float a, float b, float c,
    uint8_t* gray){


    //gray = (uint8_t*)tmp; // Temporary buffer for grayscale image

    // Grayscale conversion
    for (size_t i = 0; i < width * height; i++) {
        size_t idx = i * 3; // Each pixel has 3 components (R, G, B)
        gray[i] = (uint8_t)((a * img[idx] + b * img[idx + 1] + c * img[idx + 2]) / (a + b + c));
    }
}
