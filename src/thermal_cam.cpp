#include <Wire.h>

#include "thermal_cam.h"

#define CAM_SLAVE_ADDR 0x33

enum Regs {
    STATUS_R = 0x8000,          // Subpage config
    CONTROL_R = 0x800D,         // Read mode config register
    I2C_CONFIG_R = 0x800F,      // I2C config register
    LEFT_TOP_PIXEL_R = 0x0400   // Pixel in top left
};

void sendTwoBytes(uint16_t reg) {
    uint8_t regParts[2] = { // Seperate MSByte abd LSByte
        (uint8_t) reg & 0xFF, 
        (uint8_t) (reg >> 0xFF) & 0xFF
    };
    Wire.write(regParts, 2);
}

uint16_t camReadReg(uint16_t reg) {
    Wire.beginTransmission(CAM_SLAVE_ADDR);
    sendTwoBytes(reg); // Select register

    // Read data
    uint16_t data = 0;
    Wire.requestFrom(CAM_SLAVE_ADDR, 2);
    data |= (uint16_t) Wire.read() & 0xFF;
    data |= (uint16_t) (Wire.read() >> 0xFF) & 0xFF;
    Wire.endTransmission();
    return data;
}

void camWriteReg(uint16_t reg, uint16_t data) {
    Wire.beginTransmission(CAM_SLAVE_ADDR);
    sendTwoBytes(reg); // Select register
    
    // Send data
    sendTwoBytes(data);
    Wire.endTransmission();
}

ImageWrapper getFrame() {
    uint16_t startReg = LEFT_TOP_PIXEL_R;
    ImageWrapper imageW = {}; // NOTE: Use malloc() or struct?

    uint16_t regDiff = 0;
    for (uint8_t y = 0; y < IMAGE_HEIGHT; y++) { // Read all pixel registers
        for (uint8_t x = 0; x < IMAGE_WIDTH; x++) {

            imageW.image[y][x] = camReadReg(startReg + (regDiff++));
            // TODO: account for corrupt pixels...
        }
    }
    return imageW;
}

void thermalCamInit() {
    Wire.begin();

    // TODO: Get data...
}

void thermalCamTerminate() {
    Wire.end();

    // Todo: Turn off camera sensor?
}