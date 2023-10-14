#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define JPEG_INPUT
#define IMAGE "img/nisa.jpg"
#define OUTPUT_IMAGE "output/nisa"

// #define IMAGE "img/redblue.png"
// #define OUTPUT_IMAGE "output/redblue"
// #define IMAGE "img/heart.png"
// #define OUTPUT_IMAGE "output/heart"

#define MAX(X,Y) (((X)>(Y)) ? (X):(Y))
#define MIN(X,Y) (((X)<(Y)) ? (X):(Y))

typedef unsigned char u8;
typedef struct {
    u8 R;
    u8 B;
    u8 G;
#ifndef JPEG_INPUT
    u8 A;
#endif
} Color;
typedef float (*pixel_evaluator) (Color);

float luminance_of_pixel(Color pixel) {
    // luminance formula: https://en.wikipedia.org/wiki/Relative_luminance
    return 0.2126*pixel.R + 0.7152*pixel.G + 0.0722*pixel.B;
}
float hue_of_pixel(Color pixel) {
    // hue formula: https://www.niwa.nu/2013/05/math-behind-colorspace-conversions-rgb-hsl/
    float R = (float)pixel.R / 255.0;
    float G = (float)pixel.G / 255.0;
    float B = (float)pixel.B / 255.0;
    
    float max = MAX(R, MAX(G, B));
    float min = MIN(R, MIN(G, B));
    
    if (max == min) max++; //TODO fucked

    if (max == R)       return (G - B) / (max - min);
    else if (max == G)  return 2.0 + (B - R) / (max - min);
    else                return 4.0 + (R - G) / (max - min);
    // if (R >= G && G >= B) {
    //     if (R == B) return (G - B) / FLT_EPSILON;
    //     return (G - B) / (R - B);
    // } else if (G > R && R >= B) {
    //     if (G == B) return 2.0 - (R - B) / FLT_EPSILON;
    //     return 2.0 - (R - B) / (G - B);
    // } else if (G >= B && B > R) {
    //     if (G == R) return 2.0 + (B - R) / FLT_EPSILON;
    //     return 2.0 + (B - R) / (G - R);
    // } else if (B > G && G > R) {
    //     if (G == R) return 4.0 - (G - R) / FLT_EPSILON;
    //     return 4.0 - (G - R) / (B - R);
    // } else if (B > R && R >= G) {
    //     if (B == G) return 4.0 + (R - G) / FLT_EPSILON;
    //     return 4.0 + (R - G) / (B - G);
    // } else {
    // // } else if (R >= B && B > G) {
    //     if (R == G) return 6.0 - (B - G) / FLT_EPSILON;
    //     return 6.0 - (B - G) / (R - G);
    // }
}

void sort_buffer(Color *column, int length, pixel_evaluator eval) {
    // using insertion sort
    for (int i = 1; i < length; i++) {
        Color x = column[i];
        int j = i - 1;
        while (j >= 0 && eval(column[j]) > eval(x)) {
            column[j + 1] = column[j];
            j--;
        }
        column[j + 1] = x;
    }
}

void sort_image_vertically(Color *img, int width, int height, pixel_evaluator eval) {
    Color *column_buffer = malloc(height * sizeof(Color));
    for (int column = 0; column < width; column++) {
        for (int row = 0; row < height; row++) {
            column_buffer[row] = img[column + row * width];
        }
        sort_buffer(column_buffer, height, eval);
        for (int row = 0; row < height; row++) {
            img[column + row * width] = column_buffer[row];
        }
    }
    free(column_buffer);
}

void sort_image_horizontally(Color *img, int width, int height, pixel_evaluator eval) {
    Color *row_buffer = malloc(width * sizeof(Color));
    for (int row = 0; row < height; row++) {
        for (int column = 0; column < width; column++) {
            row_buffer[column] = img[column + row * width];
        }
        sort_buffer(row_buffer, width, eval);
        for (int column = 0; column < width; column++) {
            img[column + row * width] = row_buffer[column];
        }
    }
    free(row_buffer);
}

int main() {
    int width, height, channels;
    Color *img = (Color*)stbi_load(IMAGE, &width, &height, &channels, 0);
    if (img == NULL) {
        fprintf(stderr,"Error loading the image\n");
        exit(1);
    }
    printf("width %i height %i channels %i.\n", width, height, channels);

    // sort_image_vertically(img, width, height, &hue_of_pixel);
    sort_image_vertically(img, width, height, &luminance_of_pixel);

    if (!stbi_write_jpg(OUTPUT_IMAGE ".jpg", width, height, channels, img, 100)) {
        fprintf(stderr, "Could not create image %s.jpg\n", OUTPUT_IMAGE);
    }
    if (!stbi_write_bmp(OUTPUT_IMAGE ".bmp", width, height, channels, img)) {
        fprintf(stderr, "Could not create image %s.bmp\n", OUTPUT_IMAGE);
    }
    if (!stbi_write_png(OUTPUT_IMAGE ".png", width, height, channels, img, width * channels)) {
        fprintf(stderr, "Could not create image %s.png\n", OUTPUT_IMAGE);
    }
    stbi_image_free(img);
    return 0;
}