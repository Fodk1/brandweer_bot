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

extern "C" {
    #include "imageProcessing.h"
}

using namespace rtos;

#define START_SCAN_FLAG 0B01
#define START_TRACK_FLAG 0B10
#define NEW_FRAME_FLAG 0B100

#define motorClamp(x) {x > 1 ? 1 : (x < -1 ? -1 : x)}

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
    BLEInit();
    thermalCamInit();
    
    digitalWrite(LED_BUILTIN, LOW); // Debug

    startTimer(10000, ISR); // Debug...
    attachInterrupt(digitalPinToInterrupt(A0), ISR, FALLING);
    pinMode(A0, INPUT_PULLUP);
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
        static PidController xPID(0.2, 0.01, 0.03, 0);
        static PidController yPID(0.5, 0.01, 0.03, 0);

        static unsigned long lastUsedFrame = 0;
        flags.wait_any(START_TRACK_FLAG, osWaitForever, false);

        flags.wait_any(NEW_FRAME_FLAG);

        AllPerceivedObjs allObjs = processImage(frame);
        if (allObjs.objCount < 1) {
            if (millis() - lastUsedFrame > 1000) 
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

        // Serial.println(yPID.pid(y, timeStep));

        turretSetXMovement(xPID.pid(x, timeStep));
        turretSetYMovement(yPID.pid(y, timeStep));
        // turretSetYMovement((selObj.x-16)/32);

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

        // for (size_t i = 0; i < 24; i++)
        // {
        //     for (size_t n = 0; n < 32; n++)
        //     {
        //         Serial.println(frame[i][n]);
        //     }
        //     Serial.println();
        // }

        // ThisThread::sleep_for(1000);
        

        lastFrameUpdate = millis();
        flags.set(NEW_FRAME_FLAG);

        if (flags.get() == START_SCAN_FLAG)
            gyroUpdate(); // Gyro is not needed in track mode
        BLEUpdate();
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // Debug...
    }
}