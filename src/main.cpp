#ifdef CORE_CM7
    #include <RPC.h>
    #include "watchdog.h"
    #include "Portenta_H7_TimerInterrupt.h"

    Portenta_H7_Timer ITimer0(TIM1);

    void callback(void){
        for (size_t i = 0; i < 2; i++){
            digitalWrite(LED_BUILTIN, 1);
            delay(200);
            digitalWrite(LED_BUILTIN, 0);
            delay(200);
        }
    }

    void setup() {
        Serial.begin(115200);
        RPC.begin();

        // if(ITimer0.attachInterruptInterval(5000 * 1000, callback))
            // digitalWrite(LED_BUILTIN, 1);


        // init all systems
        // startWatchdog(2000);        
    }
  
    void loop() {

        // handle ble

        // handle PID data

        // feedWatchdog();
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

        // init all systems

    }
   
    void loop() {
        // Scan environment
        // found something? look at it with PID
        
        // go to sleep until intupted
        // LowPower.standbyM4();

        // for (size_t i = 0; i < 5; i++){
        //     digitalWrite(LED_BUILTIN, 1);
        //     delay(200);
        //     digitalWrite(LED_BUILTIN, 0);
        //     delay(200);
        // }
    }
#endif