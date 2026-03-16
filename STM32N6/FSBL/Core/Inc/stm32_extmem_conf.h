/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : stm32_extmem_conf.h
  * @version        : 1.0.0
  * @brief          : Header for extmem.c file.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32_EXTMEM_CONF__H__
#define __STM32_EXTMEM_CONF__H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/*
  @brief management of the driver layer enable
*/

#define EXTMEM_DRIVER_NOR_SFDP   1
#define EXTMEM_DRIVER_PSRAM      0
#define EXTMEM_DRIVER_SDCARD     0
#define EXTMEM_DRIVER_USER       0

/*
  @brief management of the sal layer enable
*/
#define EXTMEM_SAL_XSPI   1
#define EXTMEM_SAL_SD     0

/* Includes ------------------------------------------------------------------*/
#include "stm32n6xx_hal.h"
#include "stm32_extmem.h"
#include "stm32_extmem_type.h"
#include "boot/stm32_boot_lrun.h"
// #include "boot/stm32_boot_xip.h"

/* USER CODE BEGIN INCLUDE */

/* USER CODE END INCLUDE */
/* Private variables ---------------------------------------------------------*/
extern XSPI_HandleTypeDef hxspi2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Exported constants --------------------------------------------------------*/
/** @defgroup EXTMEM_CONF_Exported_constants EXTMEM_CONF exported constants
  * @{
  */
enum {
  EXTMEMORY_1  = 0 /*!< ID=0 for the first memory  */
};

/*
  @brief management of the boot layer
*/
// #define EXTMEM_MEMORY_BOOTXIP  EXTMEMORY_1
// #define EXTMEM_XIP_IMAGE_OFFSET 0x100000
// #define EXTMEM_HEADER_OFFSET 0x400

#define EXTMEM_LRUN_DESTINATION_INTERNAL
#define EXTMEM_LRUN_DESTINATION_ADDRESS  0x34000000


#define EXTMEM_LRUN_SOURCE EXTMEMORY_1
#define EXTMEM_LRUN_SOURCE_ADDRESS 0x00100000
#define EXTMEM_LRUN_SOURCE_SIZE 0x00010000

#define EXTMEM_HEADER_OFFSET 0x400

/* Exported configuration --------------------------------------------------------*/
/** @defgroup EXTMEM_CONF_Exported_configuration EXTMEM_CONF exported configuration definition
  * @{
  */

extern EXTMEM_DefinitionTypeDef extmem_list_config[1];
#if defined(EXTMEM_C)
EXTMEM_DefinitionTypeDef extmem_list_config[1];
#endif /* EXTMEM_C */


#ifdef __cplusplus
}
#endif

#endif /* __STM32_EXTMEM_CONF__H__ */
