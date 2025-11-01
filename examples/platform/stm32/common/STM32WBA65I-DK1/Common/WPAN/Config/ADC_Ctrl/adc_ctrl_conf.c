/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc_ctrl_conf.c
  * @author  MCD Application Team
  * @brief   Source for ADC client controller module configuration file
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

/* Includes ------------------------------------------------------------------*/
/* Own header files */
#include "adc_ctrl.h"

/* HAL ADC header */
#include "stm32wbaxx_hal_adc.h"

/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
/**
 * @brief ADC Handle configuration for LL Request temperature use
 */
ADCCTRL_Handle_t LLTempRequest_Handle =
{
  .Uid = 0x00,
  .State = ADCCTRL_HANDLE_NOT_REG,
  .InitConf =
  {
    .ClockPrescaler = ADC_CLOCK_ASYNC_DIV1,
    .Resolution = ADC_RESOLUTION_12B,
    .DataAlign = ADC_DATAALIGN_RIGHT,
    .ScanConvMode = ADC_SCAN_DISABLE,
    .EOCSelection = ADC_EOC_SINGLE_CONV,
    .LowPowerAutoPowerOff = DISABLE,
    .LowPowerAutonomousDPD = ADC_LP_AUTONOMOUS_DPD_DISABLE,
    .LowPowerAutoWait = DISABLE,
    .ContinuousConvMode = DISABLE,
    .NbrOfConversion = 1,
    .ExternalTrigConv = ADC_SOFTWARE_START,
    .ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE,
    .DMAContinuousRequests = DISABLE,
    .TriggerFrequencyMode = ADC_TRIGGER_FREQ_LOW,
    .Overrun = ADC_OVR_DATA_OVERWRITTEN,
    .SamplingTimeCommon1 = ADC_SAMPLETIME_814CYCLES_5,
    .SamplingTimeCommon2 = ADC_SAMPLETIME_1CYCLE_5,
    .OversamplingMode = DISABLE
  },
  .ChannelConf =
  {
    .Channel = ADC_CHANNEL_TEMPSENSOR,
    .Rank = ADC_REGULAR_RANK_1,
    .SamplingTime = ADC_SAMPLINGTIME_COMMON_1
  }
};
#endif /* (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1) */

/* USER CODE BEGIN User ADC configurations */

/* USER CODE END User ADC configurations */

/* Callback prototypes -------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/
/* Callback Definition -------------------------------------------------------*/
/* Private functions Definition ----------------------------------------------*/
