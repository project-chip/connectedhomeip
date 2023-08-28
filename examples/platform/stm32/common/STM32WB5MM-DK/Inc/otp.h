/**
  ******************************************************************************
  * @file    otp.h
  * @author  MCD Application Team
  * @brief   OTP manager  interface
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2018-2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __OTP_H
#define __OTP_H

#ifdef __cplusplus
extern "C" {
#endif

  /* Includes ------------------------------------------------------------------*/
#include "utilities_common.h"

  /* Exported types ------------------------------------------------------------*/
  typedef  PACKED_STRUCT
  {
    uint8_t   bd_address[6];
    uint8_t   hse_tuning;
    uint8_t   id;
  } OTP_ID0_t;

  /* Exported constants --------------------------------------------------------*/
  /* External variables --------------------------------------------------------*/
  /* Exported macros -----------------------------------------------------------*/
  /* Exported functions ------------------------------------------------------- */

  /**
   * @brief This API return the address (64 bits aligned) of the ID parameter in the OTP
   *        It returns the first ID declaration found from the higher address down to the base address
   *        The user shall fill the OTP from the base address to the top of the OTP so that the more recent
   *        declaration is returned by the API
   *        The OTP manager handles only 64bits parameter
   *        | Id    | Parameter |
   *        | 8bits | 58bits    |
   *        | MSB   |  LSB      |
   *
   * @param  id: ID of the parameter to read from OTP
   * @retval Address of the ID in the OTP - returns 0 when no ID found
   */
  uint8_t * OTP_Read( uint8_t id );

#ifdef __cplusplus
}
#endif

#endif /*__OTP_H */


