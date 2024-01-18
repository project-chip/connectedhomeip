/**
 ******************************************************************************
 * @file    otp.c
 * @author  MCD Application Team
 * @brief   OTP manager
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

/* Includes ------------------------------------------------------------------*/
#include "utilities_common.h"

#include "otp.h"

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/

uint8_t * OTP_Read(uint8_t id)
{
    uint8_t * p_id;

    p_id = (uint8_t *) (CFG_OTP_END_ADDRESS - 7);

    while (((*(p_id + 7)) != id) && (p_id != (uint8_t *) CFG_OTP_BASE_ADDRESS))
    {
        p_id -= 8;
    }

    if ((*(p_id + 7)) != id)
    {
        p_id = 0;
    }

    return p_id;
}
