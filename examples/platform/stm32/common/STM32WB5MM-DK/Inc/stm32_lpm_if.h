/* USER CODE BEGIN Header */
/**
  ******************************************************************************
* @file    stm32_lpm_if.h
* @brief   Header for stm32_lpm_if.c module (device specific LP management)
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
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32_LPM_IF_H
#define __STM32_LPM_IF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/**
  * @brief Enters Low Power Off Mode
  * @param none
  * @retval none
  */
void PWR_EnterOffMode( void );
/**
  * @brief Exits Low Power Off Mode
  * @param none
  * @retval none
  */
void PWR_ExitOffMode( void );

/**
  * @brief Enters Low Power Stop Mode
  * @note ARM exists the function when waking up
  * @param none
  * @retval none
  */
void PWR_EnterStopMode( void );
/**
  * @brief Exits Low Power Stop Mode
  * @note Enable the pll at 32MHz
  * @param none
  * @retval none
  */
void PWR_ExitStopMode( void );

/**
  * @brief Enters Low Power Sleep Mode
  * @note ARM exits the function when waking up
  * @param none
  * @retval none
  */
void PWR_EnterSleepMode( void );

/**
  * @brief Exits Low Power Sleep Mode
  * @note ARM exits the function when waking up
  * @param none
  * @retval none
  */
void PWR_ExitSleepMode( void );

#ifdef __cplusplus
}
#endif

#endif /*__STM32_LPM_IF_H */
