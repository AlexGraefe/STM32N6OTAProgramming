#include "gpio.h"

void GPIO_Config()
{
  GPIO_InitTypeDef GPIO_InitStruct_green_led = {0};

  __HAL_RCC_GPIOO_CLK_ENABLE();

  // Configure green led
  HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_RESET);
  GPIO_InitStruct_green_led.Pin = GREEN_LED_Pin;
  GPIO_InitStruct_green_led.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct_green_led.Pull = GPIO_NOPULL;
  GPIO_InitStruct_green_led.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GREEN_LED_GPIO_Port, &GPIO_InitStruct_green_led);

  HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_SET);
}