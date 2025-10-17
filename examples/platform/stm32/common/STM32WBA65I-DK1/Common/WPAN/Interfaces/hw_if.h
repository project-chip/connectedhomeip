/**
  ******************************************************************************
  * @file    hw_if.h
  * @author  MCD Application Team
  * @brief   Hardware Interface
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef HW_IF_H
#define HW_IF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32wbaxx.h"
#include "stm32wbaxx_hal_conf.h"
#include "stm32wbaxx_hal_def.h"
#include "stm32wbaxx_ll_exti.h"
#include "stm32wbaxx_ll_system.h"
#include "stm32wbaxx_ll_rcc.h"
#include "stm32wbaxx_ll_bus.h"
#include "stm32wbaxx_ll_pwr.h"
#include "stm32wbaxx_ll_cortex.h"
#include "stm32wbaxx_ll_utils.h"
#include "stm32wbaxx_ll_gpio.h"
#include "stm32wbaxx_ll_rtc.h"

/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/


/* Exported constants --------------------------------------------------------*/

/* External variables --------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/


#ifdef __cplusplus
}
#endif

#endif /*HW_IF_H */
