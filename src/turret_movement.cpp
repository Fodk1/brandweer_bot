#include <stdint.h>
#include <Arduino.h>
#include <Servo.h>

#define Y_AXIS_START_POSITION 0

uint8_t EN_PIN;
uint8_t DIR_PIN;
uint8_t MAX_ROTATION_SPEED;

Servo yAxisServo;


void turretInitXAxis(uint8_t en, uint8_t dir) {
    EN_PIN = en;
    DIR_PIN = dir;

    pinMode(DIR_PIN, OUTPUT);
    pinMode(EN_PIN, OUTPUT); 
}

void turretSetXMovement(float move){
    move = max(min(move, 1), -1);

    digitalWrite(DIR_PIN, move > 0);

    uint16_t speed = (uint8_t) abs(move * 255.0);
    
    if (speed < 10)
        speed = 0;

    analogWrite(EN_PIN, speed);
}


void turretInitYAxis(uint8_t pwm, uint8_t maxRotationSpeed) {
    yAxisServo.attach(pwm);    
    MAX_ROTATION_SPEED = maxRotationSpeed;

    yAxisServo.write(Y_AXIS_START_POSITION); // Start the servo on a known position 
}

void turretSetYMovement(float move){
    static uint8_t position = Y_AXIS_START_POSITION;

    move = max(min(move, 1), -1);

    int16_t rotation = (int16_t) (move * MAX_ROTATION_SPEED);

    position = max(min(position + rotation, 180), 0);

    yAxisServo.write(position);
}