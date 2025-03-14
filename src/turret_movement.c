#include <stdint.h>
#include <Arduino.h>

uint8_t EN_PIN;
uint8_t DIR_PIN;

void initXAxis(uint8_t en, uint8_t dir){
    EN_PIN = en;
    DIR_PIN = dir;

    pinMode(DIR_PIN, OUTPUT);
    pinMode(EN_PIN, OUTPUT);
}

void setMovement(float move){
    digitalWrite(DIR_PIN, move > 0);

    uint16_t speed = (uint8_t) abs(move * 4096.0);
    
    if (speed < 10)
        speed = 0;

    analogWrite(EN_PIN, speed);
}