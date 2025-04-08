#ifdef CORE_CM7
    #include <RPC.h>
    #include "power_state.h"

    void setup() {
        Serial.begin(115200);
        RPC.begin();
        // for (size_t i = 0; i < 2; i++){
        //     digitalWrite(LED_BUILTIN, 1);
        //     delay(500);
        //     digitalWrite(LED_BUILTIN, 0);
        //     delay(500);
        // }
        // startWatchdog(3000);
    }
  
    void loop() {
        // feedWatchdog(); 
        // delay(500);
    }
#endif

#ifdef CORE_CM4    
    #include <RPC.h>
    #include "power_state.h"

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

    void ISR(){

    }
  
    void setup() {
        SerialRPC.begin();
        startWatchdog(5000);
        __WFI();
    }
  
    /*
     * The loop() function of the M4 core processes the 'testImage' every 3 seconds,
     * the found objects are sent to the M7 core in string format.
     */
    void loop() {
        for (size_t i = 0; i < 5; i++){
            digitalWrite(LED_BUILTIN, 1);
            delay(200);
            digitalWrite(LED_BUILTIN, 0);
            delay(200);
        }
        feedWatchdog();
    }
#endif