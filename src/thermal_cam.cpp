#include <Wire.h>
#include <Arduino.h>

#include "thermal_cam.h"

#define CAM_SLAVE_ADDR 0x33

enum Regs {
    STATUS_R = 0x8000,              // Subpage config
    CONTROL_R = 0x800D,             // Read mode config register
    I2C_CONFIG_R = 0x800F,          // I2C config register
    LEFT_TOP_PIXEL_R = 0x0400,      // Pixel in top left
    PIX_CAL_CONFIG_START = 0x2440   // Start of the registers that contain pixel callibration data
};

typedef struct {
    uint8_t y;
    uint8_t x;
} Pos;

Pos defectPixels[4] = {}; // The thermal cam can have up to 4 defect pixels
uint8_t defectPixelCount = 0;

void sendTwoBytes(uint16_t reg) {
    uint8_t regParts[2] = {
        (uint8_t) ((reg >> 8) & 0xFF),
        (uint8_t) (reg & 0xFF)         
    };
    Wire.write(regParts, 2);
}

uint16_t camReadReg(uint16_t reg) {
    Wire.beginTransmission(CAM_SLAVE_ADDR);
    sendTwoBytes(reg); // Select register
    Wire.endTransmission(false);

    // Read data
    Wire.requestFrom(CAM_SLAVE_ADDR, 2);
    uint16_t data = 0;
    if (Wire.available() >= 2) {  // Ensure 2 bytes are available
        data |= (uint16_t) (Wire.read() << 8); // Read MSB
        data |= (uint16_t) Wire.read();        // Read LSB
    }
    return data;
}

void camWriteReg(uint16_t reg, uint16_t data) {
    Wire.beginTransmission(CAM_SLAVE_ADDR);
    sendTwoBytes(reg); // Select register
    
    // Send data
    sendTwoBytes(data);
    Wire.endTransmission();
}

/**
 * Get an unprocessed frame from the thermal camera
 */
ImageWrapper getFrame() {
    uint16_t startReg = LEFT_TOP_PIXEL_R;
    ImageWrapper imageW = {}; // NOTE: Use malloc(), struct or pointer from array?

    /*
     *  NOTE: Complete version of this will be more complex, adding:
     *      - Check status reg to see if new data is availible in RAM
     *      - Check control ref to see which subpage updated...
     */
    uint16_t regDiff = 0;
    for (uint8_t y = 0; y < IMAGE_HEIGHT; y++) { // Read all pixel registers
        for (uint8_t x = 0; x < IMAGE_WIDTH; x++) {
            imageW.image[IMAGE_HEIGHT - (y + 1)][x] = camReadReg(startReg + (regDiff++));
            // TODO: account for corrupt pixels...
        }
    }
    return imageW;
}

void findDefectPixel(uint8_t y, uint8_t x) {
    uint16_t currIndex = y * IMAGE_WIDTH + x;
    
    uint16_t pixCal = camReadReg(PIX_CAL_CONFIG_START + currIndex);
    if (pixCal == 0) // Check if pixel is defect
        defectPixels[defectPixelCount++] = {.y = y, .x = x};
}

void thermalCamInit() {
    Wire.begin();

    for (uint8_t y = 0; y < IMAGE_HEIGHT; y++) {
        for (uint8_t x = 0; x < IMAGE_WIDTH; x++) {
            findDefectPixel(y, x);
        }
    }
    Serial.println(defectPixelCount);
}

void thermalCamTerminate() {
    Wire.end();

    // Todo: Turn off camera sensor?
}