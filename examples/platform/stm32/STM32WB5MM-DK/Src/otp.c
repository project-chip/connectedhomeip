/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
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

    p_id = (uint8_t *) (CFG_OTP_END_ADRESS - 7);

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
