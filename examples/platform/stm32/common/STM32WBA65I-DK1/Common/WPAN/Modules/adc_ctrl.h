/**
  ******************************************************************************
  * @file    adc_ctrl.h
  * @author  MCD Application Team
  * @brief   Header for ADC client manager module
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

#ifndef ADC_CTRL_H
#define ADC_CTRL_H

/* Includes ------------------------------------------------------------------*/
/* Utilities */
#include "utilities_common.h"

/* Exported defines ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/**
 * @brief ADC command status codes
 */
typedef enum ADCCTRL_Cmd_Status
{
  ADCCTRL_OK,
  ADCCTRL_NOK,
  ADCCTRL_BUSY,
  ADCCTRL_HANDLE_ALREADY_REGISTERED,
  ADCCTRL_HANDLE_NOT_REGISTERED,
  ADCCTRL_HANDLE_NOT_VALID,
  ADCCTRL_ERROR_NULL_POINTER,
  ADCCTRL_ERROR_CONFIG,
  ADCCTRL_ERROR_STATE,
  ADCCTRL_UNKNOWN,
} ADCCTRL_Cmd_Status_t;

/**
  * @brief  ADC IP state
  */
typedef enum ADCCTRL_Ip_State
{
  ADC_OFF,
  ADC_ON
} ADCCTRL_Ip_State_t;

/**
 * @brief ADC handle states
 */
typedef enum ADCCTRL_HandleState
{
  ADCCTRL_HANDLE_NOT_REG,
  ADCCTRL_HANDLE_REG,
} ADCCTRL_HandleState_t;

/**
 * @brief ADC Init configuration structure - Conversion parameters
 */
typedef struct ADCCTRL_ConvParameters
{
  uint32_t TriggerFrequencyMode;  /*!< Set ADC Trigger frequency mode.
                                       This parameter can be a value of @ref ADC_LL_EC_REG_TRIGGER_FREQ */
  
  uint32_t Resolution;            /*!< Configure the ADC resolution.
                                       This parameter can be a value of @ref ADC_LL_EC_RESOLUTION */
  
  uint32_t DataAlign;             /*!< Specify ADC data alignment in conversion data register (right or left).
                                       This parameter can be a value of @ref ADC_LL_EC_DATA_ALIGN */
  
  uint32_t TriggerStart;          /*!< Select the source used to trigger ADC conversion
                                       start.
                                       This parameter can be a value of @ref ADC_LL_EC_REG_TRIGGER_SOURCE.*/
  
  uint32_t TriggerEdge;           /*!< Select the Edge used for trigger.
                                       This parameter can be a value of @ref ADC_LL_EC_REG_TRIGGER_EDGE.*/
  
  uint32_t ConversionMode;        /*!< Configure the conversion mode of ADC.
                                       This parameter can be a value of @ref ADC_LL_EC_REG_CONTINUOUS_MODE */

  uint32_t DmaTransfer;           /*!< DMA transfer of ADC conversion data.
                                       This parameter can be a value of @ref ADC_LL_EC_REG_DMA_TRANSFER */
  
  uint32_t Overrun;               /*!< Select the behavior in case of overrun: data overwritten or preserved (default).
                                       This parameter can be a value of @ref ADC_LL_EC_REG_OVR_DATA_BEHAVIOR. */
  
  uint32_t SamplingTimeCommon1;   /*!< Set sampling time common to a group of channels.
                                       This parameter can be a value of @ref ADC_LL_EC_CHANNEL_SAMPLINGTIME */

  uint32_t SamplingTimeCommon2;   /*!< Set sampling time common to a group of channels, second common setting possible.
                                       This parameter can be a value of @ref ADC_LL_EC_CHANNEL_SAMPLINGTIME */
}ADCCTRL_ConvParameters_t;

/**
 * @brief ADC Init configuration structure - Sequencer parameters
 */
typedef struct ADCCTRL_SeqParameters
{
  uint32_t Setup;                 /*!< Set ADC sequencer configuration flexibility.
                                       This parameter can be a value of @ref ADC_LL_EC_REG_SEQ_MODE */
  
  uint32_t Length;                /*!< Set ADC sequencer scan length.
                                       This parameter can be a value of @ref ADC_LL_EC_REG_SEQ_SCAN_LENGTH */
  
  uint32_t DiscMode;              /*!< Set ADC sequencer discontinuous mode.
                                       This parameter can be a value of @ref ADC_LL_EC_REG_SEQ_DISCONT_MODE */
}ADCCTRL_SeqParameters_t;

/**
 * @brief ADC Init configuration structure - Low power parameters
 */
typedef struct ADCCTRL_LowPowerParameters
{
  FunctionalState AutoPowerOff;   /*!< Enable or Disable ADC Auto Power OFF mode.
                                       This parameter can be set to ENABLE or DISABLE */
  
  uint32_t AutonomousDPD;         /*!< Enable or Disable ADC Autonomous Deep Power Down Mode.
                                       This parameter can be a value of @ref ADC_LL_EC_AUTONOMOUS_DEEP_POWER_DOWN_MODE */
}ADCCTRL_LowPowerParameters_t;

/**
 * @brief ADC Init configuration structure
 */
typedef struct ADCCTRL_InitConfig
{
  ADCCTRL_ConvParameters_t ConvParams;
  ADCCTRL_SeqParameters_t SeqParams;
  ADCCTRL_LowPowerParameters_t LowPowerParams;
} ADCCTRL_InitConfig_t;

/**
 * @brief ADC Channel configuration structure
 */
typedef struct ADCCTRL_ChannelConfig
{
  uint32_t Channel;                /*!< Specify the channel to configure into ADC regular group.
                                        This parameter can be a value of @ref ADC_LL_EC_CHANNEL */

  uint32_t Rank;                   /*!< Add or remove the channel from ADC regular group sequencer and specify its
                                        conversion rank.
                                        This parameter can be a value of @ref ADC_LL_EC_REG_SEQ_RANKS */

  uint32_t SamplingTime;           /*!< Sampling time value to be set for the selected channel.
                                        This parameter can be a value of @ref ADC_LL_EC_SAMPLINGTIME_COMMON */
} ADCCTRL_ChannelConfig_t;

/**
 * @brief ADC handle typedef
 */
typedef struct ADCCTRL_Handle
{
  uint32_t Uid;                           /* Id of the Handle instance */
  ADCCTRL_HandleState_t State;            /* State of the ADC Controller handle */
  ADCCTRL_InitConfig_t InitConf;          /* Init configuration of the ADC */
  ADCCTRL_ChannelConfig_t ChannelConf;    /* Channel configuration of the ADC */
} ADCCTRL_Handle_t;

/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/
/**
  * @brief  Initialize the adc controller
  *
  * @retval State of the initialization
  */
ADCCTRL_Cmd_Status_t ADCCTRL_Init(void);

/**
  * @brief  Register a ADC handle
  *
  * @param p_Handle: Handle to register
  *
  * @return State of the handle initialization
  */
ADCCTRL_Cmd_Status_t ADCCTRL_RegisterHandle (ADCCTRL_Handle_t * const p_Handle);

/**
 * @brief Request a specific state for the ADC IP - Either On or Off
 *
 * @param p_Handle: ADC handle
 * @param State: Requested state to apply
 *
 * @return State of the operation
 */
ADCCTRL_Cmd_Status_t ADCCTRL_RequestIpState (const ADCCTRL_Handle_t * const p_Handle,
                                             const ADCCTRL_Ip_State_t State);

/**
  * @brief  Read raw value of the ADC
  *
  * @param p_Handle: ADC handle
  * @param p_ReadValue: Raw ADC value
  *
  * @retval Operation state
  */
ADCCTRL_Cmd_Status_t ADCCTRL_RequestRawValue (const ADCCTRL_Handle_t * const p_Handle,
                                              uint16_t * const p_ReadValue);

/**
  * @brief  Read temperature from ADC temperature sensor
  *
  * @details The returned value is actual temperature sensor value
  *
  * @param p_Handle: ADC handle
  * @param p_ReadValue: Temperature measurement
  *
  * @retval Operation state
  */
ADCCTRL_Cmd_Status_t ADCCTRL_RequestTemperature (const ADCCTRL_Handle_t * const p_Handle,
                                                 uint16_t * const p_ReadValue);

/**
  * @brief  Read Voltage from ADC
  *
  * @details The returned value is actual Voltage value in mVolts
  *
  * @param p_Handle: ADC handle
  * @param p_ReadValue: Core Voltage measurement
  *
  * @retval Operation state
  */
ADCCTRL_Cmd_Status_t ADCCTRL_RequestCoreVoltage (const ADCCTRL_Handle_t * const p_Handle,
                                                 uint16_t * const p_ReadValue);

/**
  * @brief  Read reference Voltage from ADC
  *
  * @details The returned value is actual reference Voltage value in mVolts
  *
  * @param p_Handle: ADC handle
  * @param p_ReadValue: Reference Voltage measurement
  *
  * @retval Operation state
  */
ADCCTRL_Cmd_Status_t ADCCTRL_RequestRefVoltage (const ADCCTRL_Handle_t * const p_Handle,
                                                uint16_t * const p_ReadValue);

/* Exported functions to be implemented by the user ------------------------- */
/**
 * @brief  Take ownership on the ADC mutex
 *
 * @details This function shall be implemented by the user
 *
 * @return Status of the command
 * @retval ADCCTRL_Cmd_Status_t::ADCCTRL_OK
 * @retval ADCCTRL_Cmd_Status_t::ADCCTRL_NOK
 */
extern ADCCTRL_Cmd_Status_t ADCCTRL_MutexTake (void);

/**
 * @brief  Release ownership on the ADC mutex
 *
 * @details This function shall be implemented by the user
 *
 * @return Status of the command
 * @retval ADCCTRL_Cmd_Status_t::ADCCTRL_OK
 * @retval ADCCTRL_Cmd_Status_t::ADCCTRL_NOK
 */
extern ADCCTRL_Cmd_Status_t ADCCTRL_MutexRelease (void);

#endif /* ADC_CTRL_H */
