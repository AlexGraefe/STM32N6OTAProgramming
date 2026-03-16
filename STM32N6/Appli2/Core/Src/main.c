 /**
 ******************************************************************************
 * @file    main.c
 * @author  GPM Application Team
 *
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
#include <string.h>
#include <unistd.h>

#include "console.h"
#include "stm32n6xx_hal_gpio.h"
#include "timer.h"
#include "iris.h"
#include "fuseprogramming.h"
#include "gpio.h"
#include "iac.h"

#include "mpu.h"
#include "security.h"
#include "stm32n6xx_hal.h"
#include "stm32n6xx_hal_spi.h"
#include "system_clock.h"

#include "stm32n6570_discovery_bus.h"
#include "stm32n6570_discovery_xspi.h"
#include "stm32n6570_discovery.h"



#include "main.h"
#include <stdio.h>
#include "app_config.h"

#ifndef APP_GIT_SHA1_STRING
#define APP_GIT_SHA1_STRING "dev"
#endif
#ifndef APP_VERSION_STRING
#define APP_VERSION_STRING "unversioned"
#endif

static void Hardware_init(void);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  Hardware_init();
  while (1) {
    printf("I am the second application50000.\n");
    HAL_Delay(1000);
  }
}

static void Hardware_init(void)
{
  /* enable MPU configuration to create non cacheable sections */
  MPU_Config();

  /* Power on ICACHE */
  MEMSYSCTL->MSCR |= MEMSYSCTL_MSCR_ICACTIVE_Msk;

  SCB_EnableICache();

#if defined(USE_DCACHE)
  /* Power on DCACHE */
  MEMSYSCTL->MSCR |= MEMSYSCTL_MSCR_DCACTIVE_Msk;
  SCB_EnableDCache();
#endif

  HAL_Init();

  SystemClock_Config();

  HAL_Init();

  /* Set all required IPs as secure privileged */
  Security_Config();

  GPIO_Config();

  CONSOLE_Config();
  printf("\n\n\n\n");

  PRINTF_START("Timer Init");
  Timer_Config();
  PRINTF_END("Timer Init");

  // PRINTF_START("IRIS Init");
  // iris_config();
  // iris_handshake_blocking();
  // PRINTF_END("IRIS Init");

  Fuse_Programming();

  /* External RAM */
  // PRINTF_START("External Memory Init");
  // BSP_XSPI_RAM_Init(0);
  // BSP_XSPI_RAM_EnableMemoryMappedMode(0);

  //   BSP_XSPI_NOR_Init_t NOR_Init;
  // NOR_Init.InterfaceMode = BSP_XSPI_NOR_OPI_MODE;
  // NOR_Init.TransferRate = BSP_XSPI_NOR_DTR_TRANSFER;
  // BSP_XSPI_NOR_Init(0, &NOR_Init);

  IAC_Config();
  set_clk_sleep_mode();

}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  UNUSED(file);
  UNUSED(line);
  __BKPT(0);
  while (1)
  {
  }
}

#endif
