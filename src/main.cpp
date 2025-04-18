// Only runs on core: M7
#include <Arduino.h>
#include "rtos.h"   

#include "gyro.h"
#include "watchdog.h"
#include "turretMovement.h"
#include "timerInterrupt.h"
#include "thermalCam.h"
#include "pidController.h"
#include "BLEController.h"

#include "flags.h"
rtos::EventFlags flags;

extern "C" {
    #include "imageProcessing.h"
}

using namespace rtos;

Thread systemThread;
Thread scanThread;
Thread trackThread;

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
    BLEInit();
    thermalCamInit();

    startTimer(10000, ISR);
    attachInterrupt(digitalPinToInterrupt(A0), ISR, FALLING);
    pinMode(A0, INPUT_PULLUP);
    startWatchdog(2000);

    systemThread.start(systemUpdate);
    scanThread.start(scan);
    trackThread.start(track);

    digitalWrite(LED_BUILTIN, LOW);
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
        if (flags.get() & MANUAL_CONTROL_FLAG)
            continue;

        flags.wait_any(START_SCAN_FLAG, osWaitForever, false);

        float startPos = getXAxis();
        turretSetXMovement(-0.5);

        while (getXAxis() < startPos + 360) {

            if (flags.get() & MANUAL_CONTROL_FLAG)
                continue;
            
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
        __WFI(); // If no fire is detected wait until next interrupt
    }
}

/**
 * Track a fire if one has been detected
 */
bool track() {

    while (1)
    {
        if (flags.get() & MANUAL_CONTROL_FLAG)
            continue;

        static PidController xPID(0.2, 0.01, 0.03, 0.4);
        static PidController yPID(0.5, 0.01, 0.03, 0);

        static unsigned long lastUsedFrame = 0;
        flags.wait_any(START_TRACK_FLAG, osWaitForever, false);

        flags.wait_any(NEW_FRAME_FLAG);

        AllPerceivedObjs allObjs = processImage(frame);
        if (allObjs.objCount < 1) {
            if (millis() - lastUsedFrame > 3000) 
                flags.clear(START_TRACK_FLAG);
            turretSetXMovement(0);
            continue;
        }

        PerceivedObj* objs = allObjs.objs;

        // Select biggest object to track (most hazardous)
        PerceivedObj selObj;
        uint16_t currBiggest = 0;
        for (uint8_t i = 0; i < allObjs.objCount; i++) {
            
            if (objs[i].obj_size > currBiggest){
                currBiggest = objs[i].obj_size;
                selObj = objs[i];
            }
        }        
        free(allObjs.objs);

        float timeStep = (millis() - lastUsedFrame) / 1000.0;
        float x = ((selObj.y - (IMAGE_HEIGHT - 1) / 2.0f) / ((IMAGE_HEIGHT - 1) / 2.0f)) * -1;
        float y = ((selObj.x - (IMAGE_WIDTH - 1) / 2.0f) / ((IMAGE_WIDTH - 1) / 2.0f)) * -1;


        turretSetXMovement(xPID.pid(x, timeStep));
        turretSetYMovement(yPID.pid(y, timeStep));

        if (allObjs.objCount > 0)
            lastUsedFrame = millis();
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
        BLEUpdate();
    }
}