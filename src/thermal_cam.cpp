#include <Wire.h>
#include <math.h>
#include <Arduino.h>

extern "C" {
    #include "MLX90640_I2C_Driver.h"
    #include "MLX90640_API.h"
}

#include "thermal_cam.h"

#define CAM_SLAVE_ADDR 0x33

paramsMLX90640 mlxParams;
uint16_t eeData[832];
uint16_t frameData[834];

/**
 * Get an unprocessed frame from the thermal camera
 */
ImageWrapper getFrame() {
    MLX90640_GetFrameData(CAM_SLAVE_ADDR, frameData);

    float Ta = MLX90640_GetTa(frameData, &mlxParams);
    float emissivity = 0.95f;

    float frame[IMAGE_HEIGHT * IMAGE_WIDTH];
    MLX90640_CalculateTo(frameData, &mlxParams, emissivity, Ta, frame);

    ImageWrapper returnW;
    for (uint16_t i = 0; i < IMAGE_HEIGHT * IMAGE_WIDTH; i++) {
        uint8_t y = floor(i / IMAGE_WIDTH);
        uint8_t x = i % IMAGE_WIDTH;

        returnW.image[y][x] = frame[i];
    }
    return returnW;
}

void thermalCamInit() {
    MLX90640_I2CInit();

    MLX90640_DumpEE(CAM_SLAVE_ADDR, eeData);
    MLX90640_ExtractParameters(eeData, &mlxParams);
}

void thermalCamTerminate() {
    Wire.end();

    // Todo: Turn off camera sensor?
}