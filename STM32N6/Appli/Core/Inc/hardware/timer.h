#ifndef TIMER_H
#define TIMER_H

#include "stm32n6xx_hal.h"

void Timer_Config(void);
void my_sleep(uint32_t us);

void stopwatch_start();
void stopwatch_stop();

#endif /* TIMER_H */