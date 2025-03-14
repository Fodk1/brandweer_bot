#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H

#include <stdint.h>

#define IMAGE_HEIGHT 10
#define IMAGE_WIDTH 12

// Object that is discovered to be hot by the perception unit
typedef struct {
    float y;
    float x;
    uint16_t obj_size; // Fire size in pixels
} PerceivedObj;

// Structure with all hot objects that are detected with the amount of objects detected
typedef struct {
    PerceivedObj* objs;
    uint8_t objCount;
} AllPerceivedObjs;

/**
 * Takes an image and returns the position of dangerousely hot things in the image.
 * The size of the hot objects is also returned for priority reasons in the final code.
 */
AllPerceivedObjs processImage(const uint8_t image[][IMAGE_WIDTH]);

#endif