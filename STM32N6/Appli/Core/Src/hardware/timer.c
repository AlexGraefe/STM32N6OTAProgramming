
#include <stdint.h>
#include <stdio.h>

#include "timer.h"

// timer
TIM_HandleTypeDef htim6;
static uint32_t tim6_prescaler = 20000;   // sampling time 50us
static uint32_t stopwatch_start_cnt = 0;


/**
  * @brief Timer initialization funciton
  * @param None
  * @retval None
  */
void Timer_Config(void)
{
  // Note: input clock to timer is 400MHz 
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  // configure timer 6 as a stopwatch to measure inference time. 
  // It will run continuously and we will read the counter value before and after inference to get the time taken.
  // enable clock
  __HAL_RCC_TIM6_CLK_ENABLE();
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = tim6_prescaler - 1;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 65535;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    while(1) {}
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    while(1) {}
  }
  HAL_TIM_Base_Start(&htim6);

}

void my_sleep(uint32_t us) {
    uint16_t start_time = __HAL_TIM_GET_COUNTER(&htim6);
    uint16_t wait_time = (us / 50); // convert microseconds to timer ticks (50us per tick)
    while ((__HAL_TIM_GET_COUNTER(&htim6) - start_time) < wait_time) {
        // wait until the required time has passed
    }
}

void stopwatch_start() {
  stopwatch_start_cnt =__HAL_TIM_GET_COUNTER(&htim6);
}

void stopwatch_stop() {
  uint32_t elapsed_time = (__HAL_TIM_GET_COUNTER(&htim6) - stopwatch_start_cnt) * 50; // convert timer ticks to microseconds
  printf("%f us\n", (float)elapsed_time);
}