#include "io.h"
#include "grayscale.h"

// Sobel operator implementation
void sobel(
    const uint8_t* img, size_t width, size_t height,
    float a, float b, float c,
    void* tmp,
    uint8_t* result
) {
    /*uint8_t* gray = (uint8_t*)tmp; // Temporary buffer for grayscale image

    // Grayscale conversion
    for (size_t i = 0; i < width * height; i++) {
        size_t idx = i * 3; // Each pixel has 3 components (R, G, B)
        gray[i] = (uint8_t)((a * img[idx] + b * img[idx + 1] + c * img[idx + 2]) / (a + b + c));
    }*/
    uint8_t* gray = (uint8_t*)tmp;

    grayscale(img, width, height, a, b, c, gray);

    // Sobel edge detection
    int Mv[3][3] = {
        {1, 0, -1},
        {2, 0, -2},
        {1, 0, -1}
    };
    int Mh[3][3] = {
        { 1,  2,  1},
        { 0,  0,  0},
        {-1, -2, -1}
    };

    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            int sumV = 0, sumH = 0;

            // Apply Sobel kernel
            for (int ky = -1; ky <= 1; ky++) {
                if ((y + ky) >= 0 && (y + ky) < height)
                {
                    for (int kx = -1; kx <= 1; kx++) {
                        if ((x + kx) >= 0 && (x + kx) < width){
                            uint8_t pixel = gray[(y + ky) * width + (x + kx)];
                            sumV += Mv[ky + 1][kx + 1] * pixel;
                            sumH += Mh[ky + 1][kx + 1] * pixel;
                        }
                        else continue;
                }
                }
                else continue;
            }

            // Compute gradient magnitude
            int magnitude = (int)sqrt(sumV * sumV + sumH * sumH);
            if (magnitude > 255) magnitude = 255;
            result[y * width + x] = (uint8_t)magnitude;
        }
    }
}

// Read PPM file from filename into rgbData
void readPPM(const char* filename, int* width, int* height, uint8_t** rgbData) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Cannot open file");
        exit(1);
    }

    // Parse header
    char magic[3];
    int maxval;
    fscanf(file, "%2s", magic);
    if (strcmp(magic, "P6") != 0) {
        fprintf(stderr, "Unsupported format\n");
        fclose(file);
        exit(1);
    }
    fscanf(file, "%d %d %d", width, height, &maxval);
    fgetc(file); // Consume the newline after maxval

    // Allocate and read pixel data
    size_t pixelCount = (*width) * (*height) * 3;
    *rgbData = malloc(pixelCount);
    fread(*rgbData, 1, pixelCount, file); //try to use SIMD.
    fclose(file);
}

// Write PGM file from data to filename
void writePGM(const char* filename, const uint8_t* data, int width, int height) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        perror("Cannot open file");
        exit(1);
    }
    fprintf(file, "P5\n%d %d\n255\n", width, height);
    fwrite(data, 1, width * height, file); // try to use SIMD
    fclose(file);
}