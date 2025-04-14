// #define CORE_CM7 1 

#ifdef CORE_CM7
    #include <RPC.h>
    #include "rtos.h"
    
    #include "watchdog.h"
    #include "turretMovement.h"
    #include "timerInterrupt.h"
    // #include "sleep.h"

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
        // __HAL_RCC_HSEM_CLK_ENABLE();
        // HAL_HSEM_ActivateNotification(HSEM_ID_0); 

        turretInitXAxis(1, 0);
        turretInitYAxis(6);

        for (size_t i = 0; i < 5; i++){
            digitalWrite(LED_BUILTIN, 1);
            delay(200);
            digitalWrite(LED_BUILTIN, 0);
            delay(200);
        }

        startTimer(5000, ISR);
        
        startWatchdog(2000);
        servoThread.start(testServo);
    }
  
    void loop() {

        feedWatchdog();
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
    // #include "sleep.h"
 
    #define SerialRPC RPC
    
    void setup() {
        SerialRPC.begin();
        // HAL_NVIC_SetPriority(HSEM1_IRQn, 0, 0);
        // HAL_NVIC_EnableIRQ(HSEM1_IRQn);
        
        // gyroInit();
        // thermalCamInit();
    }
  
    void loop() {
        // enterStopmode();
        // Serial.println("Start");
        // ImageWrapper frame;
        // getFrame(&frame);
        // for (int i = 0; i < 32; i++) {
        //     Serial.println(frame.image[0][i]);
        // }
        // gyroUpdate();
        // Serial.println(getXAxis());
        delay(100);
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
    
#endif