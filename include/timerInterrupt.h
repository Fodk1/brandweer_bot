#ifndef TIMER_INTERRUPT_H
#define TIMER_INTERRUPT_H

void startTimer(uint16_t time, void (*callback)(void));

#endif