#include <Wire.h>
#include <math.h>
#include <Arduino.h>

extern "C" {
    #include "MLX90640_I2C_Driver.h"
    #include "MLX90640_API.h"
}

#include "thermalCam.h"

#define CAM_SLAVE_ADDR 0x33
#define REFRESH_RATE 32
#define EMISSIVITY 0.95f
#define READ_BUFFER_16 32
#define NONE -1

paramsMLX90640 mlxParams;
uint16_t eeData[832];
uint16_t frameData[834];

/**
 * Send two bytes to the MLX90640, this could be used to select a 16-bit indexed memory address,
 * or to change a 16-bit content of a register.
 */
void sendTwoBytes(uint16_t reg) {
    uint8_t regParts[2] = {
        (uint8_t) ((reg >> 8) & 0xFF),
        (uint8_t) (reg & 0xFF)         
    };
    Wire.write(regParts, 2);
}

uint8_t toRefreshRateCommand(float refreshRate) {
    uint8_t firstNr = floor(refreshRate);

    switch (firstNr) {
        case 0:     return 0x00;
        case 1:     return 0x01;
        case 2:     return 0x02;
        case 4:     return 0x03;
        case 8:     return 0x04;
        case 16:    return 0x05;
        case 32:    return 0x06;
        case 64:    return 0x07;
        default:    return 0x02; // Return default refresh rate
    }
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
    delay(5);
    Wire.begin();
    return 0;
}

int MLX90640_I2CRead(uint8_t slaveAddr, uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data) {
    uint16_t remaining = nMemAddressRead;
    uint16_t addr = startAddress;

    while (remaining > 0) {
        uint8_t readCount = remaining > READ_BUFFER_16 ? READ_BUFFER_16 : remaining;

        Wire.beginTransmission(slaveAddr);
        sendTwoBytes(addr);
        if (Wire.endTransmission(false) != 0) {
            Serial.println("I2C write failed");
            return -1;
        }

        uint8_t bytesToRead = readCount * 2;
        uint8_t bytesReceived = Wire.requestFrom((int)slaveAddr, (int)bytesToRead);
        if (bytesReceived != bytesToRead) {
            Serial.print("I2C read error: expected ");
            Serial.print(bytesToRead);
            Serial.print(" bytes, got ");
            Serial.println(bytesReceived);
            return -2;
        }

        for (uint8_t i = 0; i < readCount; i++) {
            data[(addr - startAddress) + i] = ((uint16_t)Wire.read() << 8) | Wire.read();
        }

        addr += readCount;
        remaining -= readCount;
    }
    return 0;
}

int MLX90640_I2CWrite(uint8_t slaveAddr, uint16_t writeAddress, uint16_t data) {
    Wire.beginTransmission((int) slaveAddr);
    sendTwoBytes(writeAddress); // Select register
    
    // Send data
    sendTwoBytes(data);
    Wire.endTransmission();

    // Check if written data is correct
    uint16_t check;
    MLX90640_I2CRead(slaveAddr, writeAddress, 1, &check);
    return check == data ? 0 : -2;
}

void MLX90640_I2CFreqSet(int freq) {
    Wire.setClock(freq);
}

void getFrame(uint16_t image[][IMAGE_WIDTH]) {
    
    int prevSubpage = -1;
    int retrievedSubpages = 0;
    
    while (retrievedSubpages < 2) {
        MLX90640_SynchFrame(CAM_SLAVE_ADDR); // Wait for frame
        int subpage = MLX90640_GetFrameData(CAM_SLAVE_ADDR, frameData);

        if (prevSubpage == subpage) continue;
        prevSubpage = subpage; // Next subpage is availible

        float Ta = MLX90640_GetTa(frameData, &mlxParams);
        float tempFrame[IMAGE_HEIGHT * IMAGE_WIDTH];
        MLX90640_CalculateTo(frameData, &mlxParams, EMISSIVITY, Ta, tempFrame);

        for (uint16_t i = 0; i < IMAGE_WIDTH * IMAGE_HEIGHT; i++) {
            uint16_t y = floor(i / IMAGE_WIDTH);
            uint16_t x = i % IMAGE_WIDTH;

            // Merge subpages in the image in their corresponding patterns
            if (subpage == 0 && ((y & 1) == (x & 1)))
                image[y][x] = tempFrame[i];
                
            if (subpage == 1 && ((y & 1) != (x & 1)))
                image[y][x] = tempFrame[i];
        }
        retrievedSubpages++;
    }
}

void thermalCamInit() {
    MLX90640_I2CInit();
    MLX90640_I2CFreqSet(1000000);

    MLX90640_I2CWrite(CAM_SLAVE_ADDR, 0x800D, 0b0001100100000001);
    MLX90640_SetRefreshRate(CAM_SLAVE_ADDR, toRefreshRateCommand(REFRESH_RATE));
    MLX90640_SetResolution(CAM_SLAVE_ADDR, 0x00);

    delay(500);

    MLX90640_DumpEE(CAM_SLAVE_ADDR, eeData);
    MLX90640_ExtractParameters(eeData, &mlxParams);
}

void thermalCamTerminate() {
    Wire.end();

    // TODO: Turn off camera sensor?
}