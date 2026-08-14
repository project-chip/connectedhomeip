/* USER CODE BEGIN Header */
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
/* USER CODE END Header */

#include "main.h"
#include "stm32wbxx_hal.h"
#include "stm32wbxx_ll_hsem.h"
#include <string.h>

int mbedtls_hardware_poll(void * data, unsigned char * output, size_t len, size_t * olen);

int mbedtls_hardware_poll(void * data, unsigned char * output, size_t len, size_t * olen)
{
    HAL_StatusTypeDef status = HAL_OK;
    uint32_t random_number   = 0;
    /**
     * Static random Address
     * The two upper bits shall be set to 1
     * The lowest 32bits is read from the UDN to differentiate between devices
     * The RNG may be used to provide a random number on each power on
     */
    /* Get RNG semaphore */
    while (LL_HSEM_1StepLock(HSEM, 0))
        ;

    /* Enable RNG */
    __HAL_RNG_ENABLE(&hrng);

    /* Enable HSI48 oscillator */
    LL_RCC_HSI48_Enable();
    /* Wait until HSI48 is ready */
    while (!LL_RCC_HSI48_IsReady())
        ;

    if (HAL_RNG_GenerateRandomNumber(&hrng, &random_number) != HAL_OK)
    {
    }

    /* Disable HSI48 oscillator */
    LL_RCC_HSI48_Disable();

    /* Disable RNG */
    __HAL_RNG_DISABLE(&hrng);

    /* Release RNG semaphore */
    LL_HSEM_ReleaseLock(HSEM, 0, 0);
    ((void) data);
    *olen = 0;

    if ((len < sizeof(uint32_t)) || (HAL_OK != status))
    {
        return 0;
    }

    memcpy(output, &random_number, sizeof(uint32_t));
    *olen = sizeof(uint32_t);

    return 0;
}
