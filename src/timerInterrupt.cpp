// Het BrandweerBot Team 16-04-2025

#include "stm32h747xx.h"
#include <Arduino.h>

void (*timerCallback)(void);

void startTimer(uint16_t milliseconds, void (*callback)(void)){

    timerCallback = callback;

    RCC->APB1LENR |= RCC_APB1LENR_TIM2EN;

    TIM2->PSC = (SystemCoreClock / 10000) - 1;  // set prescaler
    TIM2->ARR = milliseconds * 10 - 1; // set comparevalue

    TIM2->EGR |= TIM_EGR_UG; // update PSC & ARR

    TIM2->DIER |= TIM_DIER_UIE;  // Enable interrupt
    TIM2->CR1 |= TIM_CR1_CEN;    // Start timer

    NVIC_SetPriority(TIM2_IRQn, 0);
    NVIC_EnableIRQ(TIM2_IRQn);
}

extern "C" void TIM2_IRQHandler(void) {
    if (TIM2->SR & TIM_SR_UIF) {
        TIM2->SR &= ~TIM_SR_UIF;  // Clear interrupt flag
        timerCallback();
    }
}