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
    digitalWrite(DIR_PIN, 0);
    digitalWrite(EN_PIN, 0);
}

void turretSetXMovement(float move){
    move = max(min(move, 1), -1);

    digitalWrite(DIR_PIN, move > 0);

    uint16_t speed = abs(move) * 180 + 75; // everything bellow a pwm of 75/255 does not work.
    
    if (speed < 78)
        speed = 0;

    analogWrite(EN_PIN, speed);
}


void turretInitYAxis(uint8_t pwm, uint8_t maxRotationSpeed) {
    yAxisServo.attach(pwm);    
    MAX_ROTATION_SPEED = maxRotationSpeed;

    yAxisServo.write(Y_AXIS_START_POSITION); // Start the servo on a known position 
}

void turretSetYMovement(float move){
    static int position = Y_AXIS_START_POSITION;

    move = max(min(move, 1), -1);

    int rotation = (int16_t) (move * MAX_ROTATION_SPEED);

    position = max(min(position + rotation, 45), 0);

    yAxisServo.write(position);
}