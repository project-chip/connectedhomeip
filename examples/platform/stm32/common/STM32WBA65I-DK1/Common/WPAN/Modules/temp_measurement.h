/**
  ******************************************************************************
  * @file    temp_measurement.h
  * @author  MCD Application Team
  * @brief   Header for temp_measurement.c module
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
#ifndef TEMP_MEASUREMENT_H
#define TEMP_MEASUREMENT_H

/* Includes ------------------------------------------------------------------*/
#include "utilities_common.h"

/* Exported defines ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/**
 * @brief Temperature Measurement command status codes
 */
typedef enum TEMPMEAS_Cmd_Status
{
  TEMPMEAS_OK,
  TEMPMEAS_NOK,
  TEMPMEAS_ADC_INIT,
  TEMPMEAS_UNKNOWN,
} TEMPMEAS_Cmd_Status_t;

/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief  Initialize the temperature measurement
  *
  * @retval Operation state
  */
TEMPMEAS_Cmd_Status_t TEMPMEAS_Init (void);

/**
  * @brief  Request temperature measurement
  * @param  None
  * @retval None
  */
void TEMPMEAS_RequestTemperatureMeasurement (void);

#endif /* TEMP_MEASUREMENT_H */
