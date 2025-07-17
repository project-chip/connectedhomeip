/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    crc_ctrl_conf.h
  * @author  MCD Application Team
  * @brief   Configuration Header for crc_ctrl.c module
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef CRC_CTRL_CONF_H
#define CRC_CTRL_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Own header files */
#include "crc_ctrl.h"

/* CRC configuration types */
#include "stm32wbaxx_hal_crc.h"

/* Exported defines ----------------------------------------------------------*/
/**
 * @brief Physical address of the CRC to use
 */
#define CRCCTRL_HWADDR   CRC

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/**
 * @brief Handle used by the Simple NVM Arbiter to access the CRC functions
 */
extern CRCCTRL_Handle_t SNVMA_Handle;

/* Exported macros -----------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* CRC_CTRL_CONF_H */
