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

// Current mode of the robot: SCAN_MODE = deteecting fires, TRACK_MODE = tracking fires
uint8_t currMode = SCAN_MODE;

// Buffer for frame captured by the thermal camera
uint16_t frame[IMAGE_HEIGHT][IMAGE_WIDTH] = {};
unsigned long lastFrameUpdate = 0;

bool scan();
void systemUpdate();
void ISR(){
    flags.set(START_SERVO_FLAG);
    // digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200);

    // Crash test...
    for (uint8_t i = 0; i < 3; i++) {
        static uint8_t state = 0;

        digitalWrite(LED_BUILTIN, state);
        state = !state;
        delay(200);
    }
    
    turretInitXAxis(1, 0);
    turretInitYAxis(6);

    thermalCamInit();
    gyroInit();
    
    digitalWrite(LED_BUILTIN, LOW); // Debug

    startTimer(5000, ISR); // Debug...
    
    startWatchdog(2000); // Debug...

    systemThread.start(systemUpdate);
}

void loop() {
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
            delay(5000); // TODO: Sleep for 5 sec instead...
            break;

        case TRACK_MODE:
            digitalWrite(LED_BUILTIN, HIGH);
            
            Serial.println("Tracking!!!");
            break;

        default:
            Serial.println("Unknown mode...");
            exit(1);
    }
}

/**
 * Scan around the robot using the x-axis rotation to detect fires
 */
bool scan() {
    float startPos = getXAxis();
    turretSetXMovement(-0.5);

    bool hotObjFound = false;
    while (getXAxis() < startPos + 360) {
        AllPerceivedObjs objs = processImage(frame);
        free(objs.objs); // Actual objects are not needed, just the count
        
        // Check if hot object is in frame
        if (objs.objCount > 0) {
            hotObjFound = true;
            break;
        }
    }
    turretSetXMovement(0);
    return hotObjFound;
}

/**
 * Track a fire if one has been detected
 */
void track() {
    unsigned long lastFrameUsed = lastFrameUpdate;

    if (lastFrameUsed == lastFrameUpdate) 
        return; // No new frame availible

    AllPerceivedObjs allObjs = processImage(frame);
    PerceivedObj* objs = allObjs.objs;

    // Select biggest object to track (most hazardous)
    PerceivedObj selObj;
    for (uint8_t i = 0; i < allObjs.objCount; i++) {
        static uint16_t currBiggest = 0;

        if (objs[i].obj_size > currBiggest)
            selObj = objs[i];
    }
    free(objs); // Other objs no longer needed

    // Debug...
    Serial.println("Curr Tracked obj:");
    Serial.print("y = ");
    Serial.println(selObj.y);
    Serial.print("x = ");
    Serial.println(selObj.x);
    Serial.print("Size = ");
    Serial.println(selObj.obj_size);

    Serial.println();

    // TODO: implement PID
}

/*
 * Sensor reads for the scan mode
 */
void systemUpdate() {
    while(true) {
        feedWatchdog();

        getFrame(frame);
        lastFrameUpdate = millis();
        if (currMode == SCAN_MODE)
            gyroUpdate(); // Gyro is not needed in track mode
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