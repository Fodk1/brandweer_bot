#include "stm32h7xx_hal.h"

void enterStopmode(){
    HAL_HSEM_ActivateNotification(__HAL_HSEM_SEMID_TO_MASK(0)); // enable hardware semaphore


    #ifdef CORE_CM7
    HAL_PWREx_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFI, PWR_D1_DOMAIN);
    #endif

    #ifdef CORE_CM4
    HAL_PWREx_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFI, PWR_D2_DOMAIN);
    #endif
}

void wakeUp(){
    HAL_HSEM_FastTake(0); // Semaphore 0
    HAL_HSEM_Release(0, 0); // Releases to signal
}