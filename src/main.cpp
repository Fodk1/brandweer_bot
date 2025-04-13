// #define CORE_CM7 1 

#ifdef CORE_CM7
    #include <RPC.h>
    #include "rtos.h"
    
    #include "watchdog.h"
    #include "turretMovement.h"
    #include "timerInterrupt.h"
    #include "sleep.h"

    using namespace rtos;
  
    #define START_SERVO_FLAG 0x01

    Thread servoThread;
    EventFlags flags;

    void scan();
    void testServo();
    void ISR(){
        flags.set(START_SERVO_FLAG);
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }

    void setup() {
        Serial.begin(115200);
        RPC.begin();
        __HAL_RCC_HSEM_CLK_ENABLE();

        turretInitXAxis(1, 0);
        turretInitYAxis(6);


        // delay(3000);

        // startTimer(5000, ISR);
        
        // startWatchdog(2000);
        // servoThread.start(testServo);
    }
  
    void loop() {

        // feedWatchdog();
        delay(1000);
        wakeUp();
    }

    
    void scan(){
        // float startPos = getXAxis();

        // while (getXAxis() < startPos + 360)
        //     turretSetXMovement(-0.5);
        
        // turretSetXMovement(0);
    }

    void testServo(){
        while (true) {
            flags.wait_any(START_SERVO_FLAG, osWaitForever, false);

            turretSetXMovement(0.4);
            for (size_t i = 0; i < 45; i++)
            {
                turretSetYMovement(0.1);
                ThisThread::sleep_for(30);
            }
            turretSetXMovement(-0.4);

            for (size_t i = 0; i < 45; i++)
            {
                turretSetYMovement(-0.1);
                ThisThread::sleep_for(30);
            }

            turretSetXMovement(0);

            flags.clear(START_SERVO_FLAG);
        }
    }

#endif              

#ifdef CORE_CM4    
    #include <RPC.h>

    #include "thermalCam.h"
    #include "gyro.h"
    #include "sleep.h"
 
    #define SerialRPC RPC
    
    void setup() {
        SerialRPC.begin();
        // gyroInit();
        // thermalCamInit();
        for (size_t i = 0; i < 5; i++){
            digitalWrite(LED_BUILTIN, 1);
            delay(200);
            digitalWrite(LED_BUILTIN, 0);
            delay(200);
        }
    }
  
    void loop() {
        enterStopmode();
        // ImageWrapper frame;
        // getFrame(&frame);
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        delay(100);
        // gyroUpdate();
    }
    
#endif