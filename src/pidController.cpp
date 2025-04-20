// Het BrandweerBot Team 16-04-2025

#include "pidController.h"

PidController::PidController(float p, float i, float d, float setPoint) {
    this -> p = p;
    this -> i = i;
    this -> d = d;

    this -> setPoint = setPoint;
}

PidController::~PidController() {
    asm("nop"); // Nothing to deconstruct
}

float PidController::pid(float input, float timeStep) {
    float error = this -> setPoint - input;

    // Calculate derivative and integral
    float derivative = (error - this -> prevError) / timeStep;
    this -> integral += error * timeStep;

    this -> prevError = error;
    return this -> p * error + this -> i * this -> integral + this -> d * derivative;
}