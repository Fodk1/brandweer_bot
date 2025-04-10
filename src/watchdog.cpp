#include <Arduino.h>
#include "stm32h747xx.h"
// page: 2041, https://www.st.com/resource/en/reference_manual/dm00176879-stm32h745755-and-stm32h747757-advanced-armbased-32bit-mcus-stmicroelectronics.pdf

#define INDEPENDENTCLKSPEED 32000

void feedWatchdog(){
    #ifdef CORE_CM7    
        IWDG1->KR = 0xAAAA; // Reset timer key
    #endif
    #ifdef CORE_CM4    
       IWDG2->KR = 0xAAAA;
    #endif
}

void startWatchdog(uint16_t timeOut){
    #ifdef CORE_CM7    
        IWDG1->KR = 0x5555; // Enable acces to other registers key
        IWDG1->PR = 0x6; // Set prescaler
        IWDG1->RLR = min(INDEPENDENTCLKSPEED/256.0 * (timeOut-256) / 1000, 4096); // clk / prescaler * timeout in ms
        IWDG1->KR = 0xCCCC; // Enable watchdog timer key
    #endif
    #ifdef CORE_CM4    
        IWDG2->KR = 0x5555;
        IWDG2->PR = 0x6; 
        IWDG2->RLR = min(INDEPENDENTCLKSPEED/256.0 * (timeOut-256) / 1000, 4096);
        IWDG2->KR = 0xCCCC;
    #endif
    feedWatchdog();
}