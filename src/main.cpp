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

#define START_SCAN_FLAG 0B01
#define START_TRACK_FLAG 0B10
#define NEW_FRAME_FLAG 0B100

Thread systemThread;
Thread scanThread;
Thread trackThread;
EventFlags flags;


// Buffer for frame captured by the thermal camera
uint16_t frame[IMAGE_HEIGHT][IMAGE_WIDTH] = {};
unsigned long lastFrameUpdate = 0;

bool scan();
bool track();
void systemUpdate();
void ISR(){
    if (flags.get() != START_TRACK_FLAG)
        flags.set(START_SCAN_FLAG);
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200);

    turretInitXAxis(1, 0);
    turretInitYAxis(6);
    
    gyroInit();
    thermalCamInit();
    
    digitalWrite(LED_BUILTIN, LOW); // Debug

    startTimer(5000, ISR); // Debug...
    
    startWatchdog(2000); // Debug...

    systemThread.start(systemUpdate);
    scanThread.start(scan);
    trackThread.start(track);
}

void loop() {
    delay(1000);
}


/**
 * Scan around the robot using the x-axis rotation to detect fires
 */
bool scan() {
    while (1)
    {
        flags.wait_any(START_SCAN_FLAG, osWaitForever, false);

        float startPos = getXAxis();
        turretSetXMovement(-0.5);

        while (getXAxis() < startPos + 360) {
            flags.wait_any(NEW_FRAME_FLAG);
            
            AllPerceivedObjs objs = processImage(frame);
            free(objs.objs); // Actual objects are not needed, just the count
            
            // Check if hot object is in frame
            if (objs.objCount > 0) {
                flags.set(START_TRACK_FLAG);
                break;
            }
        }
        turretSetXMovement(0);
        flags.clear(START_SCAN_FLAG);
    }
}

/**
 * Track a fire if one has been detected
 */
bool track() {
    while (1)
    {
        flags.wait_any(START_TRACK_FLAG, osWaitForever, false);

        flags.wait_any(NEW_FRAME_FLAG);

        AllPerceivedObjs allObjs = processImage(frame);
        if (allObjs.objCount < 1){
            flags.clear(START_TRACK_FLAG);
            turretSetXMovement(0);
            continue;
        }
        PerceivedObj* objs = allObjs.objs;

        // Select biggest object to track (most hazardous)
        PerceivedObj selObj;
        for (uint8_t i = 0; i < allObjs.objCount; i++) {
            static uint16_t currBiggest = 0;
            
            if (objs[i].obj_size > currBiggest)
            selObj = objs[i];
        }
        free(objs); // Other objs no longer needed
        
        turretSetXMovement((12-selObj.y)/24);
    }        
}

/*
* Sensor reads for the scan mode
*/
void systemUpdate() {
    while(true) {
        feedWatchdog();
        
        getFrame(frame);
        lastFrameUpdate = millis();
        flags.set(NEW_FRAME_FLAG);

        if (flags.get() == START_SCAN_FLAG)
            gyroUpdate(); // Gyro is not needed in track mode
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // Debug...
    }
}