/**
  ******************************************************************************
  * @file    stm32wbaxx_nucleo_conf.h
  * @author  MCD Application Team
  * @brief   STM32WBAXX nucleo board configuration file.
  *          This file should be copied to the application folder and renamed
  *          to stm32wbaxx_nucleo_conf.h .
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32WBAXX_NUCLEO_CONF_H
#define STM32WBAXX_NUCLEO_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32wbaxx_hal.h"

/* Usage of nucleo board */
#define USE_NUCLEO_64      1U

/* Usage of COM feature */
#define USE_BSP_COM_FEATURE 1U
#define USE_COM_LOG         0U

/* Button interrupt priorities */
#define BSP_B1_IT_PRIORITY 0x0FUL  /* Default is lowest priority level */
#define BSP_B2_IT_PRIORITY 0x0FUL  /* Default is lowest priority level */
#define BSP_B3_IT_PRIORITY 0x0FUL  /* Default is lowest priority level */

#ifdef __cplusplus
}
#endif

#endif /* STM32WBAXX_NUCLEO_CONF_H */
