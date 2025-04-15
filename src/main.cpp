#include <RPC.h>
#include "rtos.h"

#include "watchdog.h"
#include "turretMovement.h"
#include "timerInterrupt.h"
#include "motionPlatform.h"
// #include "sleep.h"

using namespace rtos;

#define START_SERVO_FLAG 0x01

// #define MOTOR1_DIR_PIN A3
// #define MOTOR1_SPEED_PIN D5 //Fr
// #define MOTOR2_DIR_PIN A4
// #define MOTOR2_SPEED_PIN D3 //Br
// #define MOTOR3_DIR_PIN D20
// #define MOTOR3_SPEED_PIN D4 //Fl
// #define MOTOR4_DIR_PIN D21
// #define MOTOR4_SPEED_PIN D2 //Bl

#define MOTOR1_DIR_PIN D20
#define MOTOR1_SPEED_PIN D4 //Fl
#define MOTOR2_DIR_PIN A3
#define MOTOR2_SPEED_PIN D5 //Fr
#define MOTOR3_DIR_PIN D21
#define MOTOR3_SPEED_PIN D2 //Bl
#define MOTOR4_DIR_PIN A4
#define MOTOR4_SPEED_PIN D3 //Br

MotionControl motionControl(
    MOTOR1_DIR_PIN, MOTOR1_SPEED_PIN,
    MOTOR2_DIR_PIN, MOTOR2_SPEED_PIN,
    MOTOR3_DIR_PIN, MOTOR3_SPEED_PIN,
    MOTOR4_DIR_PIN, MOTOR4_SPEED_PIN
);

Thread servoThread;
Thread movementThread;
EventFlags flags;

void scan();
void testServo();
void testMovement();
void ISR(){
    flags.set(START_SERVO_FLAG);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void setup() {
    Serial.begin(115200);
    RPC.begin();

    // __HAL_RCC_HSEM_CLK_ENABLE();
    // HAL_HSEM_ActivateNotification(HSEM_ID_0); 

    turretInitXAxis(1, 0);
    turretInitYAxis(6);
    motionControl.begin();

    for (size_t i = 0; i < 5; i++){
        digitalWrite(LED_BUILTIN, 1);
        delay(200);
        digitalWrite(LED_BUILTIN, 0);
        delay(200);
    }

    startTimer(5000, ISR);
    
    startWatchdog(2000);
    servoThread.start(testServo);
    movementThread.start(testMovement);
}

void loop() {
    feedWatchdog();
}


void scan(){
    // float startPos = getXAxis();

    // while (getXAxis() < startPos + 360)
    //     turretSetXMovement(-0.5);
    
    // turretSetXMovement(0);
}

void testMovement(){
    
    // analogWrite(MOTOR1_SPEED_PIN, 100);;
    // analogWrite(MOTOR2_SPEED_PIN, 100);;
    // analogWrite(MOTOR3_SPEED_PIN, 100);;
    // analogWrite(MOTOR4_SPEED_PIN, 100);;
    // motionControl.moveVector(1,0);
    ThisThread::sleep_for(1000);
    
    motionControl.moveVector(-1,0);
    ThisThread::sleep_for(1000);

    motionControl.moveVector(0,1);
    ThisThread::sleep_for(1000);

    motionControl.moveVector(0,-1);
    ThisThread::sleep_for(1000);

    motionControl.stop();
    ThisThread::sleep_for(1000);
 
    motionControl.moveVector(1,1);
 
    ThisThread::sleep_for(1000);
    // digitalWrite(MOTOR1_DIR_PIN, 1);
    // digitalWrite(MOTOR2_DIR_PIN, 1);
    // digitalWrite(MOTOR3_DIR_PIN, 1);
    // digitalWrite(MOTOR4_DIR_PIN, 1);
    // ThisThread::sleep_for(1000);

    // digitalWrite(MOTOR1_DIR_PIN, 0);
    // digitalWrite(MOTOR2_DIR_PIN, 0);
    // digitalWrite(MOTOR3_DIR_PIN, 0);
    // digitalWrite(MOTOR4_DIR_PIN, 0);
    // ThisThread::sleep_for(1000);
    motionControl.stop();
    // ThisThread::sleep_for(1000);
    // analogWrite(MOTOR1_SPEED_PIN, 0);;
    // analogWrite(MOTOR2_SPEED_PIN, 0);;
    // analogWrite(MOTOR3_SPEED_PIN, 0);;
    // analogWrite(MOTOR4_SPEED_PIN, 0);;
}

void testServo(){
    while (true) {
        flags.wait_any(START_SERVO_FLAG, osWaitForever, false);

        turretSetXMovement(0.4);
        for (size_t i = 0; i < 45; i++)
        {
            turretSetYMovement(0.1);
            ThisThread::sleep_for(30);
        }
        turretSetXMovement(-0.4);

        for (size_t i = 0; i < 45; i++)
        {
            turretSetYMovement(-0.1);
            ThisThread::sleep_for(30);
        }

        turretSetXMovement(0);

        flags.clear(START_SERVO_FLAG);
    }
}
