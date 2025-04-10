// #define CORE_CM7 1 

#ifdef CORE_CM7
    #include <RPC.h>
    #include "gyro.h"
    #include "turretMovement.h"
  
    float startPos;

    void setup() {
        Serial.begin(115200);
        RPC.begin();

        turretInitXAxis(1, 0);
        turretInitYAxis(6);

        gyroInit();
        startPos = getXAxis();

        for (size_t i = 0; i < 2; i++){
            digitalWrite(LED_BUILTIN, 1);
            delay(200);
            digitalWrite(LED_BUILTIN, 0);
            delay(200);
        }
    }
  
    void loop() {
        if (getXAxis() > startPos + 360)
            turretSetXMovement(0);
        else
            turretSetXMovement(-0.5);
            
        if (getXAxis() > startPos + 180)
            turretSetYMovement(-0.1);
        else 
            turretSetYMovement(0.1);

        update();
    }

#endif              

#ifdef CORE_CM4    
    #include <RPC.h>

    extern "C" {
        #include "image_processing.h"
    }
 
    #define SerialRPC RPC

    
    void setup() {
        SerialRPC.begin();
    }
  
    void loop() {

    }
#endif