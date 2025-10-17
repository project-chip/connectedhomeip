/**
  ******************************************************************************
  * @file    temp_measurement.c
  * @author  MCD Application Team
  * @brief   Temp measurement module
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

/* Includes ------------------------------------------------------------------*/

/* Common utilities */
#include "utilities_common.h"

#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
/* Own header file */
#include "temp_measurement.h"
/* ADC Controller module */
#include "adc_ctrl.h"
#include "adc_ctrl_conf.h"

/* Link layer interfaces */
#include "ll_intf.h"
#include "ll_intf_cmn.h"

/* Private defines -----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Error Handler */
extern void Error_Handler(void);

/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/
TEMPMEAS_Cmd_Status_t TEMPMEAS_Init (void)
{
  TEMPMEAS_Cmd_Status_t error = TEMPMEAS_UNKNOWN;

  ADCCTRL_Cmd_Status_t eReturn = ADCCTRL_UNKNOWN;

  eReturn = ADCCTRL_RegisterHandle (&LLTempRequest_Handle);

  if ((ADCCTRL_HANDLE_ALREADY_REGISTERED == eReturn) ||
      (ADCCTRL_OK == eReturn))
  {
    error = TEMPMEAS_OK;
  }
  else
  {
    error = TEMPMEAS_ADC_INIT;
  }

  return error;
}

void TEMPMEAS_RequestTemperatureMeasurement (void)
{
  uint16_t temperature_value = 0;

  /* Enter limited critical section : disable all the interrupts with priority higher than RCC one
   * Concerns link layer interrupts (high and SW low) or any other high priority user system interrupt
   */
  UTILS_ENTER_LIMITED_CRITICAL_SECTION(RCC_INTR_PRIO<<4);

  /* Request ADC IP activation */
  ADCCTRL_RequestIpState(&LLTempRequest_Handle, ADC_ON);

  /* Get temperature from ADC dedicated channel */
  ADCCTRL_RequestTemperature (&LLTempRequest_Handle,
                              &temperature_value);

  /* Request ADC IP deactivation */
  ADCCTRL_RequestIpState(&LLTempRequest_Handle, ADC_OFF);

  /* Give the temperature information to the link layer */
  ll_intf_cmn_set_temperature_value(temperature_value);

  /* Exit limited critical section */
  UTILS_EXIT_LIMITED_CRITICAL_SECTION();
}
#endif /* USE_TEMPERATURE_BASED_RADIO_CALIBRATION */
/* Private Functions Definition ------------------------------------------------------*/
