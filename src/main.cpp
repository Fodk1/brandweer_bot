#define CORE_CM7 1
#ifdef CORE_CM7
  #include <RPC.h>
  #include "timer_interrupt.h"

    void setup() {
        Serial.begin(115200);
        RPC.begin();
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