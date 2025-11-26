/**
  ******************************************************************************
  * @file    otp.h
  * @author  MCD HW Board & MCD Application Team
  * @brief   Header file for One Time Programmable (OTP) area
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

#ifndef OTP_H
#define OTP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include "stm32wbaxx_hal.h"
#include "cmsis_compiler.h"

/* Exported types ------------------------------------------------------------*/

/*
--------------------------------------------------------------------------------
|LSB                                                                        MSB|
--------------------------------------------------------------------------------
| additional_data 8-bytes | bd_address 6 bytes | hsetune 1 byte | index 1 byte |
--------------------------------------------------------------------------------
*/
typedef __PACKED_STRUCT
{
  uint8_t additional_data[8]; /*!< 64 bits of data to fill OTP slot Ex: MB184510 */
  uint8_t bd_address[6];      /*!< Bluetooth Device Address*/
  uint8_t hsetune;            /*!< Load capacitance to be applied on HSE pad */
  uint8_t index;              /*!< Structure index */
} OTP_Data_s;

/* Exported constants --------------------------------------------------------*/

#define DEFAULT_OTP_IDX     0

/* Exported macro ------------------------------------------------------------*/
/* Exported variables ------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */

/**
 * * @brief Minimal code to use the OTP_Read function:
 *
  OTP_Data_s* otp_ptr = NULL;

  if (OTP_Read(DEFAULT_OTP_IDX, &otp_ptr) != HAL_OK) {
    return HAL_ERROR;
  }

  uint64_t bd_addr = (uint64_t) otp_ptr->bd_address[0] |
                     (uint64_t) otp_ptr->bd_address[1] << 8 |
                     (uint64_t) otp_ptr->bd_address[2] << 16 |
                     (uint64_t) otp_ptr->bd_address[3] << 24 |
                     (uint64_t) otp_ptr->bd_address[4] << 32 |
                     (uint64_t) otp_ptr->bd_address[5] << 40 ;

  uint8_t hsetune = otp_ptr->hsetune;
  uint8_t index = otp_ptr->index;
*/
HAL_StatusTypeDef OTP_Read(uint8_t index, OTP_Data_s** data);

/**
  * @brief Write OTP
  */
HAL_StatusTypeDef OTP_Write(uint8_t* additional_data, uint8_t* bd_address, uint8_t hsetune, uint8_t index);

#ifdef __cplusplus
}
#endif

#endif /* OTP_H */

