/**
  ******************************************************************************
  * @file    stm32wba65i_discovery_conf_template.h
  * @author  MCD Application Team
  * @brief   STM32WBA65I_DK1 board configuration file.
  *          This file should be copied to the application folder and renamed
  *          to stm32wba65i_discovery_conf.h .
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32WBA65I_DK1_CONF_H
#define STM32WBA65I_DK1_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32wbaxx_hal.h"

/* Usage of STM32WBA65I_DK1 board */
#define USE_STM32WBA65I_DK1                  1U

/* COM define */
#define USE_BSP_COM_FEATURE                  1U
#define USE_COM_LOG                          0U

/* Joystick Debounce Delay in ms */
#define BSP_JOY_DEBOUNCE_DELAY              200

#ifdef __cplusplus
}
#endif

#endif /* STM32WBA65I_DK1_CONF_H */

