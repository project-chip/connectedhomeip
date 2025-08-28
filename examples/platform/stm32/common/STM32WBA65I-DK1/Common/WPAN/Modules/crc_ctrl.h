/**
  ******************************************************************************
  * @file    crc_ctrl.h
  * @author  MCD Application Team
  * @brief   Header for CRC client manager module
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

#ifndef CRC_CTRL_H
#define CRC_CTRL_H

/* Includes ------------------------------------------------------------------*/
/* Utilities */
#include "utilities_common.h"

/* HAL CRC header */
#include "stm32wbaxx_hal_crc.h"

#ifdef __cplusplus
extern "C" {
#endif
/* Exported defines ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/**
 * @brief CRC command status codes
 */
typedef enum CRCCTRL_Cmd_Status
{
  CRCCTRL_OK,
  CRCCTRL_NOK,
  CRCCTRL_BUSY,
  CRCCTRL_HANDLE_ALREADY_REGISTERED,
  CRCCTRL_HANDLE_NOT_REGISTERED,
  CRCCTRL_HANDLE_NOT_VALID,
  CRCCTRL_ERROR_NULL_POINTER,
  CRCCTRL_ERROR_CONFIG,
  CRCCTRL_UNKNOWN,
} CRCCTRL_Cmd_Status_t;

/**
 * @brief CRC command status codes
 */
typedef enum CRCCTRL_HandleState
{
  HANDLE_NOT_REG,
  HANDLE_REG,
} CRCCTRL_HandleState_t;

/**
 * @brief CRC configuration structure
 *
 */
typedef struct CRCCTRL_Config
{
  uint8_t DefaultPolynomialUse;       /*!< This parameter is a value of @ref CRC_Default_Polynomial and indicates if default polynomial is used.
                                            If set to DEFAULT_POLYNOMIAL_ENABLE, resort to default
                                            X^32 + X^26 + X^23 + X^22 + X^16 + X^12 + X^11 + X^10 +X^8 + X^7 + X^5 +
                                            X^4 + X^2+ X +1.
                                            In that case, there is no need to set GeneratingPolynomial field.
                                            If otherwise set to DEFAULT_POLYNOMIAL_DISABLE, GeneratingPolynomial and
                                            CRCLength fields must be set. */

  uint8_t DefaultInitValueUse;        /*!< This parameter is a value of @ref CRC_Default_InitValue_Use and indicates if default init value is used.
                                           If set to DEFAULT_INIT_VALUE_ENABLE, resort to default
                                           0xFFFFFFFF value. In that case, there is no need to set InitValue field. If
                                           otherwise set to DEFAULT_INIT_VALUE_DISABLE, InitValue field must be set. */

  uint32_t GeneratingPolynomial;      /*!< Set CRC generating polynomial as a 7, 8, 16 or 32-bit long value for a polynomial degree
                                           respectively equal to 7, 8, 16 or 32. This field is written in normal,
                                           representation e.g., for a polynomial of degree 7, X^7 + X^6 + X^5 + X^2 + 1
                                           is written 0x65. No need to specify it if DefaultPolynomialUse is set to
                                            DEFAULT_POLYNOMIAL_ENABLE.   */

  uint32_t CRCLength;                 /*!< This parameter is a value of @ref CRC_Polynomial_Sizes and indicates CRC length.
                                           Value can be either one of
                                           @arg @ref CRC_POLYLENGTH_32B                  (32-bit CRC),
                                           @arg @ref CRC_POLYLENGTH_16B                  (16-bit CRC),
                                           @arg @ref CRC_POLYLENGTH_8B                   (8-bit CRC),
                                           @arg @ref CRC_POLYLENGTH_7B                   (7-bit CRC). */

  uint32_t InitValue;                 /*!< Init value to initiate CRC computation. No need to specify it if DefaultInitValueUse
                                           is set to DEFAULT_INIT_VALUE_ENABLE.   */

  uint32_t InputDataInversionMode;    /*!< This parameter is a value of @ref CRCEx_Input_Data_Inversion and specifies input data inversion mode.
                                           Can be either one of the following values
                                           @arg @ref CRC_INPUTDATA_INVERSION_NONE       no input data inversion
                                           @arg @ref CRC_INPUTDATA_INVERSION_BYTE       byte-wise inversion, 0x1A2B3C4D
                                           becomes 0x58D43CB2
                                           @arg @ref CRC_INPUTDATA_INVERSION_HALFWORD   halfword-wise inversion,
                                           0x1A2B3C4D becomes 0xD458B23C
                                           @arg @ref CRC_INPUTDATA_INVERSION_WORD       word-wise inversion, 0x1A2B3C4D
                                           becomes 0xB23CD458 */

  uint32_t OutputDataInversionMode;   /*!< This parameter is a value of @ref CRCEx_Output_Data_Inversion and specifies output data (i.e. CRC) inversion mode.
                                            Can be either
                                            @arg @ref CRC_OUTPUTDATA_INVERSION_DISABLE   no CRC inversion,
                                            @arg @ref CRC_OUTPUTDATA_INVERSION_ENABLE    CRC 0x11223344 is converted
                                             into 0x22CC4488 */

  uint32_t InputDataFormat;           /*!< This parameter is a value of @ref CRC_Input_Buffer_Format and specifies input data format.
                                            Can be either
                                            @arg @ref CRC_INPUTDATA_FORMAT_BYTES       input data is a stream of bytes
                                            (8-bit data)
                                            @arg @ref CRC_INPUTDATA_FORMAT_HALFWORDS   input data is a stream of
                                            half-words (16-bit data)
                                            @arg @ref CRC_INPUTDATA_FORMAT_WORDS       input data is a stream of words
                                            (32-bit data)

                                          Note that constant CRC_INPUT_FORMAT_UNDEFINED is defined but an initialization
                                          error must occur if InputBufferFormat is not one of the three values listed
                                          above  */
} CRCCTRL_Config_t;

typedef struct CRCCTRL_Handle
{
  uint32_t Uid;                           /* Id of the Handle instance */
  uint32_t PreviousComputedValue;         /* Previous CRC computed value for Accumulate purposes */
  CRCCTRL_HandleState_t State;            /* State of the CRC Controller handle */
  CRCCTRL_Config_t Configuration;         /* Configuration of the CRC */
} CRCCTRL_Handle_t;

/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/
/**
  * @brief  Init the CRC Ctrl variables
  *
  * @return State of the handle initialization
  */
CRCCTRL_Cmd_Status_t CRCCTRL_Init (void);

/**
  * @brief  Register a CRC handle
  *
  * @param p_Handle: Handle to register
  *
  * @return State of the handle registration
  */
CRCCTRL_Cmd_Status_t CRCCTRL_RegisterHandle (CRCCTRL_Handle_t * const p_Handle);

/**
  * @brief  Compute the CRC of the given payload
  *
  * @param p_Handle: CRC handle
  * @param a_Payload: Address of the first payload element
  * @param PayloadSize: Size of the payload to compute the CRC
  * @param p_ComputedValue: Computed CRC (returned value LSBs for CRC shorter than 32 bits)
  *
  * @return State of the operation
  */
CRCCTRL_Cmd_Status_t CRCCTRL_Calculate (CRCCTRL_Handle_t * const p_Handle,
                                        uint32_t a_Payload[],
                                        const uint32_t PayloadSize,
                                        uint32_t * const p_ConmputedValue);

/**
  * @brief  Keep computing the CRC of a given payload
  *
  * @details Computation starts with the previous computed CRC as initialization value.
  *
  * @details The CRCCTRL_Calculate shall be used before any call of this API.
  *
  * @param p_Handle: Address of the first payload element
  * @param a_Payload: Address of the first payload element
  * @param PayloadSize: Size of the payload to compute the CRC
  * @param p_ComputedValue: Computed CRC (returned value LSBs for CRC shorter than 32 bits)
  *
  * @return State of the operation
  */
CRCCTRL_Cmd_Status_t CRCCTRL_Accumulate (CRCCTRL_Handle_t * const p_Handle,
                                         uint32_t a_Payload[],
                                         const uint32_t PayloadSize,
                                         uint32_t * const p_ConmputedValue);

/* Exported functions to be implemented by the user ------------------------- */
/**
 * @brief  Take ownership on the CRC mutex
 *
 * @details This function shall be implemented by the user
 *
 * @return Status of the command
 * @retval CRCCTRL_Cmd_Status_t::CRCCTRL_OK
 * @retval CRCCTRL_Cmd_Status_t::CRCCTRL_NOK
 */
extern CRCCTRL_Cmd_Status_t CRCCTRL_MutexTake (void);

/**
 * @brief  Release ownership on the CRC mutex
 *
 * @details This function shall be implemented by the user
 *
 * @return Status of the command
 * @retval CRCCTRL_Cmd_Status_t::CRCCTRL_OK
 * @retval CRCCTRL_Cmd_Status_t::CRCCTRL_NOK
 */
extern CRCCTRL_Cmd_Status_t CRCCTRL_MutexRelease (void);

#ifdef __cplusplus
}
#endif
#endif /* CRC_CTRL_H */
