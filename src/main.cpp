#ifdef CORE_CM7
    #include <RPC.h>
    #include "rtos.h"

    using namespace rtos;

    Thread led_thread;

    void blinky() {
        while (true) {
            digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
            
            ThisThread::sleep_for(500);
        }
    }

    void setup() {
        Serial.begin(115200);
        RPC.begin();
        for (size_t i = 0; i < 3; i++){
            digitalWrite(LED_BUILTIN, 1);
            delay(200);
            digitalWrite(LED_BUILTIN, 0);
            delay(200);
        }

        led_thread.start(blinky);
    }
  
    void loop() {
    }
#endif

#ifdef CORE_CM4    
    #include <RPC.h>
    #include "Arduino_LowPowerPortentaH7.h"

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