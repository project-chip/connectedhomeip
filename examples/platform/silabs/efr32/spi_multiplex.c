/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "dmadrv.h"
#include "em_bus.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_ldma.h"
#include "em_usart.h"
#include "spi_multiplex.h"

void SPIDRV_ReInit(uint32_t baudrate)
{
    if (USART_BaudrateGet(USART0) == baudrate)
    {
        // USART synced to baudrate already
        return;
    }
    // USART is used in MG24 + WF200 combination
    USART_InitSync_TypeDef usartInit = USART_INITSYNC_DEFAULT;
    usartInit.msbf                   = true;
    usartInit.clockMode              = usartClockMode0;
    usartInit.baudrate               = baudrate;
    uint32_t databits      = SL_SPIDRV_FRAME_LENGTH - 4U + _USART_FRAME_DATABITS_FOUR;
    usartInit.databits     = (USART_Databits_TypeDef) databits;
    usartInit.autoCsEnable = true;

    USART_InitSync(USART0, &usartInit);
}
