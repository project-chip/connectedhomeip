/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *      Source implementation of an input / output stream for MW320 targets.
 */

#include <lib/shell/Engine.h>
#include <lib/shell/streamer.h>

#include <fcntl.h>
#include <lib/core/CHIPError.h>
#include <stdio.h>
#include <string.h>

#ifdef MW320_SHELL_STREAMER

#include "board.h"
#include "clock_config.h"
#include "fsl_uart.h"
#include "pin_mux.h"

#define DEMO_LPUART UART0
#define DEMO_LPUART_CLK_FREQ BOARD_DEBUG_UART_CLK_FREQ
#ifndef BOARD_DEBUG_UART_BAUDRATE
#define BOARD_DEBUG_UART_BAUDRATE (115200U)
#endif /* BOARD_DEBUG_UART_BAUDRATE */

namespace chip {
namespace Shell {

int streamer_mw320_init(streamer_t * streamer)
{
    uart_config_t config;
    /*
     * config.baudRate_Bps = 115200U;
     * config.parityMode = kLPUART_ParityDisabled;
     * config.stopBitCount = kLPUART_OneStopBit;
     * config.txFifoWatermark = 0;
     * config.rxFifoWatermark = 0;
     * config.enableTx = false;
     * config.enableRx = false;
     */
    UART_GetDefaultConfig(&config);
    config.baudRate_Bps               = BOARD_DEBUG_UART_BAUDRATE;
    config.enable                     = true;
    config.fifoConfig.txFifoWatermark = kUART_TxfifoHalfEmpty;
    config.fifoConfig.rxFifoWatermark = kUART_RxfifoByte1;
    UART_Init(DEMO_LPUART, &config, DEMO_LPUART_CLK_FREQ);

    return 0;
}

int streamer_mw320_read(streamer_t * streamer, char * buf, size_t len)
{
    if (UART_ReadBlocking(DEMO_LPUART, (uint8_t *) buf, len) == 0)
    {
        return len;
    }
    else
    {
        return 0;
    }
}

int streamer_mw320_write(streamer_t * streamer, const char * buf, size_t len)
{
    if (UART_WriteBlocking(DEMO_LPUART, (const unsigned char *) buf, len) == 0)
    {
        return len;
    }
    else
    {
        return 0;
    }
}

static streamer_t streamer_mw320 = {
    .init_cb  = streamer_mw320_init,
    .read_cb  = streamer_mw320_read,
    .write_cb = streamer_mw320_write,
};

streamer_t * streamer_get(void)
{
    return &streamer_mw320;
}

} // namespace Shell
} // namespace chip

#endif // #ifdef MW320_SHELL_STREAMER
