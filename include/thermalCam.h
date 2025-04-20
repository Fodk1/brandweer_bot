#ifndef THERMAL_CAM_H
#define THERMAL_CAM_H

#include <stdint.h>

#define IMAGE_HEIGHT 24
#define IMAGE_WIDTH 32

typedef struct {
    uint16_t image[24][32];
} ImageWrapper;

/**
 * @brief Update frame buffer sent in argument
 * @param image 2D image buffer (IMAGE_HEIGHT x IMAGE_WIDTH)
 */
void getFrame(uint16_t image[][IMAGE_WIDTH]);
void thermalCamInit();

#endif