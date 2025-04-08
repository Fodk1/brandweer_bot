#ifndef TURRET_MOVEMENT_H
#define TURRET_MOVEMENT_H

#include <stdint.h>

/**
 * @brief Set the enable and direction pin for the X-axis motor.
 * 
 * @param en PWM pin
 * @param dir Digital pin
 */
void turretInitXAxis(uint8_t en, uint8_t dir);

/**
 * @brief Set the movement direction of the X-axis: 1 go right, -1 go left.
 * 
 * @param move Number from 1 to -1
 */
void turretSetXMovement(float move);

/**
 * @brief Set the PWM pin for the Y-axis motor.
 * 
 * @param pwm PWM pin
 * @param maxRotationSpeed The maximum rotation in degrees per update;
 */
void turretInitYAxis(uint8_t pwm, uint8_t maxRotationSpeed = 10);

/**
 * @brief Set the movement direction of the Y-axis: 1 go up, -1 go down.
 * 
 * @param move Number from 1 to -1
 */
void turretSetYMovement(float move);



#endif