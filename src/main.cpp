// Only runs on core: M7
#include <Arduino.h>
#include "rtos.h"

#include "gyro.h"
#include "watchdog.h"
#include "turretMovement.h"
#include "timerInterrupt.h"
#include "thermalCam.h"

using namespace rtos;

#define START_SERVO_FLAG 0x01

enum {
    SCAN_MODE,
    TRACK_MODE
};

// Thread servoThread;
EventFlags flags;

// Buffer for frame captured by the thermal camera
uint16_t frame[IMAGE_HEIGHT][IMAGE_WIDTH] = {};

void scan();
// void testServo();
void ISR(){
    flags.set(START_SERVO_FLAG);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void setup() {
    Serial.begin(115200);

    turretInitXAxis(1, 0);
    turretInitYAxis(6);

    gyroInit();

    // Debug...
    for (size_t i = 0; i < 2; i++){
        digitalWrite(LED_BUILTIN, 1);
        delay(200);
        digitalWrite(LED_BUILTIN, 0);
        delay(200);
    }
    
    startTimer(5000, ISR); // Debug...
    
    startWatchdog(2000); // Debug...
}

void loop() {
    // gyroUpdate();
    feedWatchdog();
    // delay(100);

    uint8_t currMode = SCAN_MODE;

    switch(currMode) {
        case SCAN_MODE:
            scan();
            startTimer(5000, ISR); // Sleep for 5 seconds
            break;

        case TRACK_MODE:
            break;

        default:
            Serial.println("Unknown mode...");
            exit(1);
    }
}


void scan() {
    float startPos = getXAxis();
    while (getXAxis() < startPos + 360) {
        
    }
    
    turretSetXMovement(0);
}

// void testServo(){
//     while (true) {
//         flags.wait_any(START_SERVO_FLAG, osWaitForever, false);

//         turretSetXMovement(0.4);
//         for (size_t i = 0; i < 45; i++)
//         {
//             turretSetYMovement(0.1);
//             ThisThread::sleep_for(30);
//         }
//         turretSetXMovement(-0.4);

//         for (size_t i = 0; i < 45; i++)
//         {
//             turretSetYMovement(-0.1);
//             ThisThread::sleep_for(30);
//         }

//         turretSetXMovement(0);

//         flags.clear(START_SERVO_FLAG);
//     }
// }