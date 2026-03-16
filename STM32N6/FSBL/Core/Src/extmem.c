/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : mx_extmem.c    test dom
  * @version        : 1.0.0
  * @brief          : This file implements the Secure Manager Api
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "extmem.h"

/* Private variables ---------------------------------------------------------*/
#define HEADER_V2_1_IMG_SIZE_OFFSET 76
#define HEADER_V2_3_IMG_SIZE_OFFSET 108
#define HEADER_V2_1_SIZE 1024
#define HEADER_V2_3_SIZE 1024

uint32_t BOOT_GetApplicationSize(uint32_t img_addr)
{
  uint32_t img_size;

  img_size = (*(uint32_t *)(img_addr + HEADER_V2_3_IMG_SIZE_OFFSET));
  img_size += HEADER_V2_3_SIZE;

  return img_size;
}

