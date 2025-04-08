#ifdef CORE_CM7
  #include <RPC.h>
  #include "turret_movement.h"

    void setup() {
        Serial.begin(115200);
        turretInitXAxis(1, 0);
        turretInitYAxis(6);
        RPC.begin();

        for (size_t i = 0; i < 2; i++){
            digitalWrite(LED_BUILTIN, 1);
            delay(500);
            digitalWrite(LED_BUILTIN, 0);
            delay(500);
        }
    }
  
    /*
     * The loop() function of the M7 core only prints "M7: hello" every second and prints
     * the objects calculated by the M4 core when they are availible
     */
    void loop() {
        Serial.println("M7: Hello");
        while (RPC.available()) { // Get the objects from the M4 core
            Serial.write(RPC.read()); 
        }
        turretSetXMovement(0.7);
        delay(2000);
        turretSetXMovement(0);
        
        delay(1000);

        for (size_t i = 0; i < 18; i++)
        {
            turretSetYMovement(1);
            delay(100);
        }
        
        delay(1000);
              
        turretSetXMovement(-0.7);
        delay(2000);
        turretSetXMovement(0);

        delay(1000);

        for (size_t i = 0; i < 18; i++)
        {
            turretSetYMovement(-1);
            delay(100);
        }

        delay(1000);

        turretSetXMovement(0.7);
        for (size_t i = 0; i < 18; i++)
        {
            turretSetYMovement(1);
            delay(100);
        }
        turretSetXMovement(0);

        delay(1000);

        turretSetXMovement(-0.7);
        for (size_t i = 0; i < 18; i++)
        {
            turretSetYMovement(-1);
            delay(100);
        }
        turretSetXMovement(0);

        delay(1000);
    }
#endif              

#ifdef CORE_CM4    
    #include <RPC.h>

    extern "C" {
        #include "image_processing.h"
    }
 
    #define SerialRPC RPC

    uint8_t testImage[10][12] = { // Image to test processing on (no camera availible yet)
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 100, 100, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 100, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 99, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 99, 0, 0, 0, 0, 0},
        {0, 0, 0, 5, 4, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 100, 100, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 100, 100, 0, 0},
        {0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0, 0}
    };
  
    void setup() {
        SerialRPC.begin();
    }
  
    /*
     * The loop() function of the M4 core processes the 'testImage' every 3 seconds,
     * the found objects are sent to the M7 core in string format.
     */
    void loop() {
        AllPerceivedObjs allObjs = processImage(testImage); // Calculate targets from image
        
        char returnStr[676];
        for (uint8_t i = 0; i < allObjs.objCount; i++) { // Print all found objects
            sprintf(returnStr,"Obj %u: y = %f, x = %f, size = %u.", i, allObjs.objs[i].y, allObjs.objs[i].x, allObjs.objs[i].obj_size);
            SerialRPC.println(returnStr); 
        }
        delay(3000);
    }
#endif