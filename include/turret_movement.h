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

#endif