// The Brandweer Bot team

#include <MLX90640_I2C_Driver.h>
#include <Wire.h>

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