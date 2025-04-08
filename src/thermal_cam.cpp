#include <Wire.h>
#include <math.h>
#include <Arduino.h>

#include "thermal_cam.h"

#define CAM_SLAVE_ADDR 0x33

enum Regs {
    STATUS_R = 0x8000,              // Subpage config
    CONTROL_R = 0x800D,             // Read mode config register
    I2C_CONFIG_R = 0x800F,          // I2C config register
    LEFT_TOP_PIXEL_R = 0x0400,      // Pixel in top left
    PIX_CAL_CONFIG_START = 0x2440,  // Start of the registers that contain pixel callibration data

    VDD_PARAM_R = 0x2433,           // VDD Parameters
    PTAT_PARAM_R = 0x2432,
    PTAT_25_R = 0x2431,    
    SCALE_R = 0x2410,
    OFFSET_AVERAGE_R = 0x2411,
    OCC_ROW_START = 0x2412,
    OCC_COL_START = 0x2418,
    ACC_ROW_START = 0x2422,
    ACC_COL_START = 0x2428,
    PIX_OFFSET_START = 0x2440,
};

// Callibration parameters
float kvdd = 0;
float vdd25 = 0;
float ta = 0;

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

/*
* Callibration Functions
*
* These functions will retrieve the callibration data stored in the registers and RAM
* of the camera. These calculations are completely based on the calculations defined
* in the datasheet, therefore the naming of variabeles will also be the same:
* 
* https://www.melexis.com/en/documents/documentation/datasheets/datasheet-mlx90640
*/

inline int16_t convertSigned(uint16_t val, uint8_t bits) {
    uint16_t max = 1 << bits;
    return (int16_t) (val > (max >> 1) - 1 ? val - max : val);
}

uint16_t getChunk(uint16_t startReg, uint8_t stepSize, uint8_t steps) {
    uint8_t regOffset = floor((steps - 1) / (16 / stepSize));
    uint8_t bitOffset = ((steps - 1) % stepSize) * stepSize;

    uint16_t selectionMask = ((1 << stepSize) - 1) << bitOffset;
    return camReadReg(startReg + regOffset) & selectionMask;
}

void restoreVDD() {
    uint16_t vddReg = camReadReg(VDD_PARAM_R);

    kvdd = (float) convertSigned(vddReg & 0xFF00, 8);
    kvdd *= pow(2, 5);

    vdd25 = (float) (vddReg & 0x00FF);
    vdd25 = (vdd25 - 256) * pow(2, 5) - pow(2, 13);
}

// Restore the ambient temperature in Celsius
void restoreTa() {
    uint16_t ptatReg = camReadReg(PTAT_PARAM_R);

    float kvPTAT = (float) convertSigned((ptatReg & 0xFC00) / pow(2, 10), 6);
    kvPTAT = kvPTAT / pow(2, 12);

    float ktPTAT = (float) convertSigned(ptatReg & 0x03FF, 10);
    ktPTAT /= pow(2, 3);

    float dV = (float) (camReadReg(0x072A)); // RAM
    dV = (dV - vdd25) / kvdd; // NOTE: Kv not defined (kvdd could be wrong)
    
    float vPTAT = (float) convertSigned(camReadReg(0x0720), 16); // RAM
    float vBE = (float) convertSigned(camReadReg(0x0700), 16); // RAM

    float alphaPTAT_EE = (float) (camReadReg(SCALE_R) & 0xF000);
    alphaPTAT_EE /= pow(2, 12);

    float alphaPTAT = alphaPTAT_EE / pow(2, 2) + 8;

    float vPTAT25 = (float) convertSigned(camReadReg(PTAT_25_R), 16);

    float vPTATart = (vPTAT / (vPTAT * alphaPTAT + vBE)) * pow(2,18);

    // Ambient temp in Celsius
    ta = (vPTATart / (1 + kvPTAT * dV) - vPTAT25) / ktPTAT + 25;
}

// Restore the offset of a pixel where i = y and j = x
float restoreOffet(uint8_t i, uint8_t j) {
    static float offsetAverage = (float) convertSigned(camReadReg(OFFSET_AVERAGE_R), 16);
    
    float occRow = (float) convertSigned(getChunk(OCC_ROW_START, 4, i), 4);
    float occScaleRow = (float) ((camReadReg(SCALE_R) & 0x0F00) >> 8);

    float occCol = convertSigned(getChunk(OCC_COL_START, 4, j), 4);
    float occScaleCol = (float) ((camReadReg(SCALE_R) & 0x00F0) >> 4);

    uint16_t offsetInput = camReadReg(PIX_OFFSET_START + (IMAGE_WIDTH * i) + j) & 0xFC00;
    float offsetIJ = (float) (convertSigned(offsetInput, 6) >> 10);

    float occScaleRemnant = (float) (camReadReg(SCALE_R) & 0x000F);

    return offsetAverage + occRow * 2 * occScaleRow + occCol * 2 * occScaleCol + offsetIJ * 2 * occScaleRemnant;
}

float restoreSensitivity(uint8_t i, uint8_t j) {
    
}

void thermalCamInit() {
    Wire.begin();

    restoreVDD();
    restoreTa();

    // TODO: find data
    Serial.print("VDD = ");
    Serial.println(kvdd);

    Serial.print("Ta = ");
    Serial.println(ta);
}

void thermalCamTerminate() {
    Wire.end();

    // Todo: Turn off camera sensor?
}