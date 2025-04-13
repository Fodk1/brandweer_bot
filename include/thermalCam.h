#ifndef THERMAL_CAM_H
#define THERMAL_CAM_H

#include <stdint.h>

#define IMAGE_HEIGHT 24
#define IMAGE_WIDTH 32

typedef struct {
    uint16_t image[24][32];
} ImageWrapper;

void getFrame(ImageWrapper* frame);
void thermalCamInit();

void updateSubpages();

#endif