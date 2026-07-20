/**
 ******************************************************************************
 * @file    main.h
 * @author  MCD Application Team
 * @brief   Header for main.c module
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2019-2021 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32wbxx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#ifdef USE_STM32WBXX_USB_DONGLE
#include "stm32wbxx_usb_dongle.h"
#endif
#ifdef USE_STM32WBXX_NUCLEO
#include "stm32wbxx_nucleo.h"
#endif
#ifdef USE_X_NUCLEO_EPD
#include "x_nucleo_epd.h"
#endif
#ifdef USE_STM32WB5M_DK
#include "stm32wb5mm_dk.h"

#endif

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern RTC_HandleTypeDef hrtc; /**< RTC handler declaration */
extern RNG_HandleTypeDef hrng;

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#endif /* __MAIN_H */
