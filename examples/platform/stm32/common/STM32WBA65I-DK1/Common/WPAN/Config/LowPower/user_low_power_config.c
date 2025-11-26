/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    user_low_power_config.c
  * @author  MCD Application Team
  * @brief   Low power related user configuration.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
#include "user_low_power_config.h"
#include "stdint.h"
#include "app_conf.h"
#include "stm32wbaxx.h"

/* Exported constants --------------------------------------------------------*/

/* Table of CPU peripheral registers to backup/restore when standby mode available */
const volatile uint32_t* const register_backup_table[] =
{
  &(SCB->CPACR),     /* FPU SCB->CPACR register */
  &(SysTick->CTRL),  /* SysTick CTRL register */
  &(SysTick->LOAD),  /* SysTick LOAD register */
  &(SysTick->VAL),   /* SysTick VAL register */
  &(SCB->VTOR),      /* SCB->VTOR register */

  /* USER CODE BEGIN RBL */

  /* USER CODE END RBL */
};

/* Table of registers size */
const uint32_t register_backup_table_size = (sizeof(register_backup_table)/sizeof(register_backup_table[0]));

/* USER CODE BEGIN EC */

/* USER CODE END EC */
