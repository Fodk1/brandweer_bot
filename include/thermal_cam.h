#ifndef THERMAL_CAM_H
#define THERMAL_CAM_H

#include <stdint.h>

#define IMAGE_HEIGHT 10
#define IMAGE_WIDTH 12

typedef struct {
    uint8_t image[24][32];
} ImageWrapper;

ImageWrapper getFrame();
void thermalCamInit();

#endif