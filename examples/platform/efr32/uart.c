/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include "uart.h"
#include "em_core.h"
#include "em_usart.h"
#include "hal-config.h"
#include "init_board.h"
#include "uartdrv.h"
#include <retargetserial.h>
#include <stddef.h>

void uartConsoleInit(void)
{
    RETARGET_SerialCrLf(0);
    RETARGET_SerialInit();
    initVcomEnable();
}

int16_t uartConsoleWrite(const char * Buf, uint16_t BufLength)
{
    if (Buf == NULL || BufLength < 1)
    {
        return -1;
    }

    for (int i = 0; i < BufLength; i++)
    {
        RETARGET_WriteChar(Buf[i]);
    }
    return (int16_t) BufLength;
}

int16_t uartConsoleRead(char * Buf, uint16_t BufLength)
{
    if (Buf == NULL || BufLength < 1)
    {
        return -1;
    }

    for (int i = 0; i < BufLength; i++)
    {
        int readVal = -1;
        while (readVal == -1)
        {
            readVal = RETARGET_ReadChar();
        }
        Buf[i] = (char) readVal;
    }
    return (int16_t) BufLength;
}
