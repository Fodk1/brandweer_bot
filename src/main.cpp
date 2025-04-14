// Only runs on core: M7
#include <Arduino.h>
#include "rtos.h"

#include "gyro.h"
#include "watchdog.h"
#include "turretMovement.h"
#include "timerInterrupt.h"
#include "thermalCam.h"

extern "C" {
    #include "imageProcessing.h"
}

using namespace rtos;

#define START_SERVO_FLAG 0x01

enum {
    SCAN_MODE,
    TRACK_MODE
};

Thread systemThread;
EventFlags flags;

// Buffer for frame captured by the thermal camera
uint16_t frame[IMAGE_HEIGHT][IMAGE_WIDTH] = {};

bool scan();
void systemUpdate();
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

    systemThread.start(systemUpdate);
}

void loop() {
    static uint8_t currMode = SCAN_MODE;

    switch(currMode) {
        /*
         * In Scan Mode; the BB will check for fires in a circle around it.
         * If it finds an actice fire, it's mode will change to Scan Mode.
         */
        case SCAN_MODE:
            if (scan()) {
                currMode = TRACK_MODE; // Hot object in frame, start tracking it
                break;
            }
            delay(5000);
            // startTimer(5000, ISR); // not Sleep for 5 seconds
            break;

        case TRACK_MODE:
            Serial.println("Tracking!!!");
            break;

        default:
            Serial.println("Unknown mode...");
            exit(1);
    }
}


bool scan() {
    float startPos = getXAxis();
    turretSetXMovement(-0.5);

    while (getXAxis() < startPos + 360) {
        getFrame(frame);

        // Check if hot object is in frame
        if (processImage(frame).objCount > 0) 
            return true; // Hot object detected
    }
    
    turretSetXMovement(0);
    return false; // No hot object detected
}

void systemUpdate(){
    while(1){
        gyroUpdate();
        feedWatchdog();
    }
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