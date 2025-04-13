#ifndef MOTION_PLATFORM_H
#define MOTION_PLATFORM_H
#include <Arduino.h>
#include <math.h>

class MotionControl
{
private:
    int directionPins[4];
    int speedPins[4];
    int pwmChannels[4];
    int maxSpeed;
    bool isMoving;
    float currentX;
    float currentY;

public:
    MotionControl(
        int m1Dir, int m1Speed,
        int m2Dir, int m2Speed,
        int m3Dir, int m3Speed,
        int m4Dir, int m4Speed)
    {
        directionPins[0] = m1Dir;
        directionPins[1] = m2Dir;
        directionPins[2] = m3Dir;
        directionPins[3] = m4Dir;
        speedPins[0] = m1Speed;
        speedPins[1] = m2Speed;
        speedPins[2] = m3Speed;
        speedPins[3] = m4Speed;
        // Assign PWM channels (0-3) for the four motors
        pwmChannels[0] = 0;
        pwmChannels[1] = 1;
        pwmChannels[2] = 2;
        pwmChannels[3] = 3;
        maxSpeed = 255;
        isMoving = false;
        currentX = 0;
        currentY = 0;
    }

    void begin()
    {
        for (int i = 0; i < 4; i++)
        {
            // Set up direction pins as digital outputs
            pinMode(directionPins[i], OUTPUT);
            digitalWrite(directionPins[i], LOW);
            // Configure PWM for speed pins
            ledcSetup(pwmChannels[i], 5000, 8); // 5kHz, 8-bit resolution
            ledcAttachPin(speedPins[i], pwmChannels[i]);
            ledcWrite(pwmChannels[i], 0); // Start with motors off
        }
    }

    void moveVector(float xComponent, float yComponent)
    {
        const float DEADZONE = 0.01;

        // Check if the vector is within the deadzone
        if (fabs(xComponent) < DEADZONE && fabs(yComponent) < DEADZONE)
        {
            stop();
            return;
        }

        // Normalize the vector to maintain direction but use full power
        float length = sqrt(xComponent * xComponent + yComponent * yComponent);
        float normalizedX = xComponent / length;
        float normalizedY = yComponent / length;

        // Calculate motor speeds based on the normalized vector
        int motorSpeeds[4];
        motorSpeeds[0] = (normalizedY - normalizedX) * maxSpeed;
        motorSpeeds[1] = (normalizedY + normalizedX) * maxSpeed;
        motorSpeeds[2] = (normalizedY + normalizedX) * maxSpeed;
        motorSpeeds[3] = (normalizedY - normalizedX) * maxSpeed;

        // Apply motor speeds
        for (int i = 0; i < 4; i++)
        {
            digitalWrite(directionPins[i], motorSpeeds[i] >= 0 ? HIGH : LOW);
            ledcWrite(pwmChannels[i], abs(motorSpeeds[i]));
        }

        isMoving = true;
        currentX = normalizedX;
        currentY = normalizedY;
    }

    void rotate(bool clockwise = true)
    {
        for (int i = 0; i < 4; i++)
        {
            boolean direction = ((i % 2) == 0) ? clockwise : !clockwise;
            digitalWrite(directionPins[i], direction ? HIGH : LOW);
            ledcWrite(pwmChannels[i], maxSpeed);
        }

        isMoving = true;
        currentX = 0;
        currentY = 0;
    }

    void stop()
    {
        for (int i = 0; i < 4; i++)
        {
            digitalWrite(directionPins[i], LOW);
            ledcWrite(pwmChannels[i], 0);
        }

        isMoving = false;
        currentX = 0;
        currentY = 0;
    }

    float getDirectionX()
    {
        return currentX;
    }

    float getDirectionY()
    {
        return currentY;
    }

    bool isActive()
    {
        return isMoving;
    }
};
#endif
