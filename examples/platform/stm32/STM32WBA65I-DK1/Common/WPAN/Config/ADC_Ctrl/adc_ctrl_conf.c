/* USER CODE BEGIN Header */
/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License,
 * Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy
 * of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to
 * in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *
 * limitations under the License.
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
/* Own header files */
#include "adc_ctrl_conf.h"
#include "adc_ctrl.h"

/* LL ADC header */
#include "stm32wbaxx_ll_adc.h"

/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
/**
 * @brief ADC Handle configuration for LL Request temperature use
 */
ADCCTRL_Handle_t LLTempRequest_Handle = {
    .Uid         = 0x00,
    .State       = ADCCTRL_HANDLE_NOT_REG,
    .InitConf    = { .ConvParams     = { .TriggerFrequencyMode = LL_ADC_TRIGGER_FREQ_LOW,
                                         .Resolution           = LL_ADC_RESOLUTION_12B,
                                         .DataAlign            = LL_ADC_DATA_ALIGN_RIGHT,
                                         .TriggerStart         = LL_ADC_REG_TRIG_SOFTWARE,
                                         .TriggerEdge          = LL_ADC_REG_TRIG_EXT_RISING,
                                         .ConversionMode       = LL_ADC_REG_CONV_SINGLE,
                                         .DmaTransfer          = LL_ADC_REG_DMA_TRANSFER_NONE,
                                         .Overrun              = LL_ADC_REG_OVR_DATA_OVERWRITTEN,
                                         .SamplingTimeCommon1  = LL_ADC_SAMPLINGTIME_814CYCLES_5,
                                         .SamplingTimeCommon2  = LL_ADC_SAMPLINGTIME_1CYCLE_5 },
                     .SeqParams      = { .Setup    = LL_ADC_REG_SEQ_CONFIGURABLE,
                                         .Length   = LL_ADC_REG_SEQ_SCAN_DISABLE,
                                         .DiscMode = LL_ADC_REG_SEQ_DISCONT_DISABLE },
                     .LowPowerParams = { .AutoPowerOff = DISABLE, .AutonomousDPD = LL_ADC_LP_AUTONOMOUS_DPD_DISABLE } },
    .ChannelConf = { .Channel = LL_ADC_CHANNEL_TEMPSENSOR, .Rank = LL_ADC_REG_RANK_1, .SamplingTime = LL_ADC_SAMPLINGTIME_COMMON_1 }
};
#endif /* (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1) */

/* USER CODE BEGIN User ADC configurations */

/* USER CODE END User ADC configurations */

/* Callback prototypes -------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/
/* Callback Definition -------------------------------------------------------*/
/* Private functions Definition ----------------------------------------------*/
