#ifndef WATCHDOG_H
#define WATCHDOG_H

/**
 * @brief Start watchdog timer that reset the whole system if timedout
 * 
 * @param timeOut in milli seconds to maximum of 32767.
 */
void startWatchdog(uint16_t timeOut);
void feedWatchdog();

#endif