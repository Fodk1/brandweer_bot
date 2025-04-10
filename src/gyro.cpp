#include <Wire.h>

// I2C-adres
#define MPU6050_ADDR 0x68  

// MPU-6050 registers
#define PWR_MGMT_1    0x6B  
#define ACCEL_XOUT_H  0x3B  
#define ACCEL_XOUT_L  0x3C  
#define ACCEL_YOUT_H  0x3D  
#define ACCEL_YOUT_L  0x3E  
#define ACCEL_ZOUT_H  0x3F  
#define ACCEL_ZOUT_L  0x40  
#define GYRO_XOUT_H   0x43  
#define GYRO_XOUT_L   0x44  
#define GYRO_YOUT_H   0x45  
#define GYRO_YOUT_L   0x46  
#define GYRO_ZOUT_H   0x47  
#define GYRO_ZOUT_L   0x48

float gyroOffset = 0;
float xRotation;

int16_t readSensor(byte reg) {
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU6050_ADDR, 2, true);

    byte highByte = Wire.read();
    byte lowByte = Wire.read();

    // Combineer de hoge en lage byte tot een 16-bit waarde
    int16_t value = (highByte << 8) | lowByte;

    return value;
}

void calibrateGyro() {
    
    float sum = 0;
    const int numSamples = 200;
    
    for(int i = 0; i < numSamples; i++) {
        sum += readSensor(GYRO_ZOUT_H);
        delay(1);
    }
    
    gyroOffset = sum / numSamples;
}

void gyroInit(){
    Wire.begin();
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(PWR_MGMT_1);
    Wire.write(0);
    Wire.endTransmission(true);

    calibrateGyro();
}

float getXAxis() {
    return xRotation;
}

void update(){
    static long prevUpdate = 0;
    long currentTime = millis();
    float elapsedTime = (currentTime - prevUpdate) / 1000.0;
    prevUpdate = currentTime;
    
    int16_t gyroRaw = readSensor(GYRO_ZOUT_H);
    float gyroValue = (gyroRaw - gyroOffset) / 131.0;
    
    xRotation += gyroValue * elapsedTime;
}