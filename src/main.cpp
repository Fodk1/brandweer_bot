// #define CORE_CM7 1
#ifdef CORE_CM7
    #include <RPC.h>
    #include "timer_interrupt.h"

    void callback(){
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }

    void setup() {
        Serial.begin(115200);
        RPC.begin();
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        delay(1000);
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        delay(1000);
        startTimer(1000, callback);
    }
  
    void loop() {
        delay(1000);
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