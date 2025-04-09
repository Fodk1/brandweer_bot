#include <Wire.h>
#include <math.h>
#include <Arduino.h>

extern "C" {
    #include "MLX90640_I2C_Driver.h"
    #include "MLX90640_API.h"
}

#include "thermal_cam.h"

#define CAM_SLAVE_ADDR 0x33
#define EMISSIVITY 0.95f
#define REFRESH_RATE 8

paramsMLX90640 mlxParams;
uint16_t eeData[832];
uint16_t frameData[834];

uint16_t subpage0[834];
uint16_t subpage1[834];

void sendTwoBytes(uint16_t reg) {
    uint8_t regParts[2] = {
        (uint8_t) ((reg >> 8) & 0xFF),
        (uint8_t) (reg & 0xFF)         
    };
    Wire.write(regParts, 2);
}

uint16_t camReadReg(uint8_t slaveAddr, uint16_t reg) {
    Wire.beginTransmission((int) slaveAddr);
    sendTwoBytes(reg); // Select register
    Wire.endTransmission(false);

    // Read data
    Wire.requestFrom((int) slaveAddr, 2);
    uint16_t data = 0;
    if (Wire.available() >= 2) {  // Ensure 2 bytes are available
        data |= (uint16_t) (Wire.read() << 8); // Read MSB
        data |= (uint16_t) Wire.read();        // Read LSB
    }
    return data;
}

void camWriteReg(uint8_t slaveAddr, uint16_t reg, uint16_t data) {
    Wire.beginTransmission((int) slaveAddr);
    sendTwoBytes(reg); // Select register
    
    // Send data
    sendTwoBytes(data);
    Wire.endTransmission();
}

/*
* MLX90640 driver implementation:
* To use the MLX90640 library, functions for I2C communication should be implemented
* for the driver.
*/

void MLX90640_I2CInit() {
    Wire.begin();
}

int MLX90640_I2CGeneralReset() {
    Wire.end();
    Wire.begin();
    return 0;
}

int MLX90640_I2CRead(uint8_t slaveAddr, uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data) {
    for (uint16_t i = 0; i < nMemAddressRead; i++) {
        uint16_t readData = camReadReg(slaveAddr, startAddress + i);
        data[i] = readData;
    }
    return 0;
}

int MLX90640_I2CWrite(uint8_t slaveAddr, uint16_t writeAddress, uint16_t data) {
    camWriteReg(slaveAddr, writeAddress, data);

    // Check if written data is correct
    uint16_t check;
    MLX90640_I2CRead(slaveAddr, writeAddress, 1, &check);
    return check == data ? 0 : -2;
}

void MLX90640_I2CFreqSet(int freq) {
    Wire.setClock(freq);
}

uint8_t refreshRateToComm(float refreshRate) {
    uint8_t firstDig = floor(refreshRate);

    switch(firstDig) {
        case 0:     return 0x00;
        case 1:     return 0x01;
        case 2:     return 0x02;
        case 4:     return 0x03;
        case 8:     return 0x04;
        case 16:    return 0x05;
        case 32:    return 0x06;
        case 64:    return 0x07;
        default:    return 0x02; // Return default
    }
}

void updateSubpages() {
    static unsigned long lastSubpage0 = 0;
    static unsigned long lastSubpage1 = 0;

    static bool firstCall = true;

    unsigned long currTime = millis();

    if (firstCall) { // First call, synchronise timers
        MLX90640_SynchFrame(CAM_SLAVE_ADDR);
        lastSubpage0 = currTime;
        lastSubpage1 = currTime + REFRESH_RATE;

        firstCall = false;
    }
    if (currTime > lastSubpage0 + 2 * REFRESH_RATE) { // New subpage0 availible
        int test = MLX90640_GetFrameData(CAM_SLAVE_ADDR, subpage0);
        lastSubpage0 = currTime;
    }
    if (currTime > lastSubpage1 + 2 * REFRESH_RATE) { // New subpage1 availible
        unsigned long t = millis();
        int test = MLX90640_GetFrameData(CAM_SLAVE_ADDR, subpage1);
        unsigned long diff = millis() - t;
        Serial.println(diff);

        lastSubpage1 = currTime;
    }
}

/**
 * Get an unprocessed frame from the thermal camera
 */
ImageWrapper getFrame() {

    ImageWrapper frame;
    int prevSubpage = -1;
    int retrievedSubpages = 0;

    while (retrievedSubpages < 2) {
        int subpage = MLX90640_GetFrameData(CAM_SLAVE_ADDR, frameData);
        // Serial.println(subpage);

        if (prevSubpage == subpage) continue;
        prevSubpage = subpage;

        float Ta = MLX90640_GetTa(frameData, &mlxParams);
        float tempFrame[IMAGE_HEIGHT * IMAGE_WIDTH];
        MLX90640_CalculateTo(frameData, &mlxParams, EMISSIVITY, Ta, tempFrame);

        for (uint16_t i = 0; i < IMAGE_WIDTH * IMAGE_HEIGHT; i++) {
            uint16_t y = floor(i / IMAGE_WIDTH);
            uint16_t x = i % IMAGE_WIDTH;

            // Store in Chess pattern from subpage
            if (subpage == 0 && ((y & 1) == (x & 1))) {
                frame.image[y][x] = tempFrame[i];
            }
            if (subpage == 1 && ((y & 1) != (x & 1))) {
                frame.image[y][x] = tempFrame[i];
            }
        }
        retrievedSubpages++;
    }
    return frame;
}

void thermalCamInit() {
    MLX90640_I2CInit();
    MLX90640_I2CFreqSet(800000);
    MLX90640_SetRefreshRate(CAM_SLAVE_ADDR, refreshRateToComm(REFRESH_RATE));

    delay(1000);

    MLX90640_DumpEE(CAM_SLAVE_ADDR, eeData);
    MLX90640_ExtractParameters(eeData, &mlxParams);
}

void thermalCamTerminate() {
    Wire.end();

    // Todo: Turn off camera sensor?
}