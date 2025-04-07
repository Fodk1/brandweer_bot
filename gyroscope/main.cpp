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

void setup() {
  Serial.begin(9600);
  Wire.begin(21, 22);

  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(PWR_MGMT_1);
  Wire.write(0);
  Wire.endTransmission(true);

  delay(100);
}

void loop() {
  
  int16_t accelX = readSensor(ACCEL_XOUT_H);
  int16_t accelY = readSensor(ACCEL_YOUT_H);
  int16_t accelZ = readSensor(ACCEL_ZOUT_H);

  int16_t gyroX = readSensor(GYRO_XOUT_H);
  int16_t gyroY = readSensor(GYRO_YOUT_H);
  int16_t gyroZ = readSensor(GYRO_ZOUT_H);

  // Omrekenen naar Gs/s
  float ax = accelX / 16384.0;
  float ay = accelY / 16384.0;
  float az = accelZ / 16384.0;

  // Omrekenen naar °/s
  float gx = gyroX / 131.0;
  float gy = gyroY / 131.0;
  float gz = gyroZ / 131.0;

  Serial.print("Accel (g) X: "); Serial.print(ax);
  Serial.print(" Y: "); Serial.print(ay);
  Serial.print(" Z: "); Serial.println(az);

  Serial.print("Gyro (°/s) X: "); Serial.print(gx);
  Serial.print(" Y: "); Serial.print(gy);
  Serial.print(" Z: "); Serial.println(gz);

  delay(500);
}

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
