/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright 2023 NXP
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
 *      Source implementation of an input / output stream for NXP targets.
 */

/* -------------------------------------------------------------------------- */
/*                                  Includes                                  */
/* -------------------------------------------------------------------------- */

#include <lib/shell/Engine.h>
#include <lib/shell/streamer.h>

#include <stdio.h>
#include <string.h>

#include "board.h"
#include "fsl_clock.h"
#include "fsl_component_serial_manager.h"
#include "fsl_os_abstraction.h"

#if CHIP_ENABLE_OPENTHREAD
#include "openthread-system.h"
#endif

/* -------------------------------------------------------------------------- */
/*                               Private macros                               */
/* -------------------------------------------------------------------------- */
#ifndef STREAMER_UART_SERIAL_MANAGER_RING_BUFFER_SIZE
#define STREAMER_UART_SERIAL_MANAGER_RING_BUFFER_SIZE (128U)
#endif
#ifndef STREAMER_UART_FLUSH_DELAY_MS
#define STREAMER_UART_FLUSH_DELAY_MS 2
#endif

#ifndef BOARD_APP_UART_INSTANCE
#if defined(RW612_SERIES) || defined(RW610_SERIES)
#define BOARD_APP_UART_INSTANCE 3
#else
#define BOARD_APP_UART_INSTANCE 1
#endif
#endif
#ifndef BOARD_APP_UART_CLK_FREQ
#if defined(RW612_SERIES) || defined(RW610_SERIES)
#define BOARD_APP_UART_CLK_FREQ CLOCK_GetFlexCommClkFreq(3)
#else
#define BOARD_APP_UART_CLK_FREQ BOARD_BT_UART_CLK_FREQ
#endif
#endif

// pw RPC uses UART DMA by default
#ifdef PW_RPC_ENABLED
#define CONSUMER_TASK_HANDLE RpcTaskHandle
#ifndef STREAMER_UART_USE_DMA
#define STREAMER_UART_USE_DMA 1
#endif
#else
#define CONSUMER_TASK_HANDLE AppMatterCliTaskHandle
#ifndef STREAMER_UART_USE_DMA
#define STREAMER_UART_USE_DMA 0
#endif
#endif // PW_RPC_ENABLED

#if STREAMER_UART_USE_DMA
typedef serial_port_uart_dma_config_t streamer_serial_port_uart_config_t;
#else
typedef serial_port_uart_config_t streamer_serial_port_uart_config_t;
#endif

/* -------------------------------------------------------------------------- */
/*                             Private prototypes                             */
/* -------------------------------------------------------------------------- */

static void Uart_RxCallBack(void * pData, serial_manager_callback_message_t * message, serial_manager_status_t status);
static void Uart_TxCallBack(void * pBuffer, serial_manager_callback_message_t * message, serial_manager_status_t status);

/* -------------------------------------------------------------------------- */
/*                               Private memory                               */
/* -------------------------------------------------------------------------- */

static SERIAL_MANAGER_HANDLE_DEFINE(streamerSerialHandle);
static SERIAL_MANAGER_WRITE_HANDLE_DEFINE(streamerSerialWriteHandle);
static SERIAL_MANAGER_READ_HANDLE_DEFINE(streamerSerialReadHandle);
static volatile int txCount   = 0;
volatile static bool readDone = true;

static streamer_serial_port_uart_config_t uartConfig = { .clockRate    = 0,
                                                         .baudRate     = BOARD_DEBUG_UART_BAUDRATE,
                                                         .parityMode   = kSerialManager_UartParityDisabled,
                                                         .stopBitCount = kSerialManager_UartOneStopBit,
                                                         .enableRx     = 1,
                                                         .enableTx     = 1,
                                                         .enableRxRTS  = 0,
                                                         .enableTxCTS  = 0,
                                                         .instance     = BOARD_APP_UART_INSTANCE,
#if STREAMER_UART_USE_DMA
                                                         .dma_instance = 0,
                                                         .rx_channel   = 1,
                                                         .tx_channel   = 0
#endif
};

static uint8_t s_ringBuffer[STREAMER_UART_SERIAL_MANAGER_RING_BUFFER_SIZE];
static const serial_manager_config_t s_serialManagerConfig = {
    .ringBuffer     = &s_ringBuffer[0],
    .ringBufferSize = STREAMER_UART_SERIAL_MANAGER_RING_BUFFER_SIZE,
#if STREAMER_UART_USE_DMA
    .type = kSerialPort_UartDma,
#else
    .type = BOARD_DEBUG_UART_TYPE,
#endif
    .blockType  = kSerialManager_NonBlocking,
    .portConfig = (serial_port_uart_config_t *) &uartConfig,
};

OSA_MUTEX_HANDLE_DEFINE(streamerMutex);

/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */

namespace chip {
namespace Shell {

int streamer_nxp_init(streamer_t * streamer)
{
    (void) streamer;
    serial_manager_status_t status = kStatus_SerialManager_Error;

#if defined(RW612_SERIES) || defined(RW610_SERIES)
    /* attach FRG3 clock to FLEXCOMM3 */
    BOARD_CLIAttachClk();
#endif

    uartConfig.clockRate = BOARD_APP_UART_CLK_FREQ;

#if STREAMER_UART_USE_DMA
    dma_channel_mux_configure_t dma_channel_mux;
    dma_channel_mux.dma_dmamux_configure.dma_tx_channel_mux = kDmaRequestLPUART1Tx;
    dma_channel_mux.dma_dmamux_configure.dma_rx_channel_mux = kDmaRequestLPUART1Rx;
    uartConfig.dma_channel_mux_configure                    = &dma_channel_mux;
#endif

    /*
     * Make sure to disable interrupts while initializating the serial manager interface
     * Some issues could happen if a UART IRQ is firing during serial manager initialization
     */
    OSA_InterruptDisable();

    do
    {
        if (SerialManager_Init((serial_handle_t) streamerSerialHandle, &s_serialManagerConfig) != kStatus_SerialManager_Success)
            break;
        if (SerialManager_OpenWriteHandle((serial_handle_t) streamerSerialHandle,
                                          (serial_write_handle_t) streamerSerialWriteHandle) != kStatus_SerialManager_Success)
            break;
        if (SerialManager_OpenReadHandle((serial_handle_t) streamerSerialHandle, (serial_read_handle_t) streamerSerialReadHandle) !=
            kStatus_SerialManager_Success)
            break;
        if (SerialManager_InstallRxCallback((serial_read_handle_t) streamerSerialReadHandle, Uart_RxCallBack, NULL) !=
            kStatus_SerialManager_Success)
            break;
        if (SerialManager_InstallTxCallback((serial_write_handle_t) streamerSerialWriteHandle, Uart_TxCallBack, NULL) !=
            kStatus_SerialManager_Success)
            break;
        status = kStatus_SerialManager_Success;
    } while (0);

    OSA_InterruptEnable();

    osa_status_t status_osa = OSA_MutexCreate((osa_mutex_handle_t) streamerMutex);
    assert(status_osa == KOSA_StatusSuccess);

    return status;
}

ssize_t streamer_nxp_read(streamer_t * streamer, char * buffer, size_t length)
{
    uint32_t bytesRead             = 0;
    serial_manager_status_t status = kStatus_SerialManager_Success;

    if (length != 0)
    {
        /**
         * If the reading process is over,
         * let CLI Task enter blocked state until notification
         **/
        if (readDone)
        {
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            readDone = false;
        }

        status = SerialManager_TryRead((serial_read_handle_t) streamerSerialReadHandle, (uint8_t *) buffer, length, &bytesRead);
        assert(status != kStatus_SerialManager_Error);

        /**
         * In certain cases such as a copy-paste of multiple commands, we may encounter '\n' or '\r' caracters
         * although the buffer is not empty yet, so the reading process should be considered done only when the
         * bytesRead return null,
         * this is to ensure that all commands are processed before blocking the CLI task
         *
         **/
        if (bytesRead == 0)
        {
            readDone = true;
        }
    }

    return bytesRead;
}

ssize_t streamer_nxp_write(streamer_t * streamer, const char * buffer, size_t length)
{
    uint32_t intMask;
    serial_manager_status_t status = kStatus_SerialManager_Success;
    size_t len                     = 0;

    /* Mutex lock to ensure the streamer write is accessed by only one task at a time */
    osa_status_t status_osa = OSA_MutexLock(streamerMutex, osaWaitForever_c);

    // If length is 0 there will be an assert in Serial Manager. Some OT functions output 0 bytes, for example
    // in SrpServer::Process<Cmd("service")> -> OutputLine(hasSubType ? "" : "(null)");
    if (length > 0)
    {
        intMask = DisableGlobalIRQ();
        txCount++;
        status = SerialManager_WriteNonBlocking((serial_write_handle_t) streamerSerialWriteHandle, (uint8_t *) buffer,
                                                (uint32_t) length);
        EnableGlobalIRQ(intMask);
        if (status == kStatus_SerialManager_Success)
        {
            len = length;
        }

        /* Wait for the serial manager task to empty the TX buffer */
        while (txCount)
        {
            OSA_TimeDelay(STREAMER_UART_FLUSH_DELAY_MS);
        }
    }

    status_osa = OSA_MutexUnlock(streamerMutex);
    assert(status_osa == KOSA_StatusSuccess);

    return len;
}

static streamer_t streamer_nxp = {
    .init_cb  = streamer_nxp_init,
    .read_cb  = streamer_nxp_read,
    .write_cb = streamer_nxp_write,
};

streamer_t * streamer_get(void)
{
    return &streamer_nxp;
}

} // namespace Shell
} // namespace chip

/* -------------------------------------------------------------------------- */
/*                              Private functions                             */
/* -------------------------------------------------------------------------- */
extern TaskHandle_t CONSUMER_TASK_HANDLE;

static void Uart_RxCallBack(void * pData, serial_manager_callback_message_t * message, serial_manager_status_t status)
{
    if (CONSUMER_TASK_HANDLE != NULL)
    {
        /* notify the main loop that a RX buffer is available */
        xTaskNotifyGive(CONSUMER_TASK_HANDLE);
    }
}

static void Uart_TxCallBack(void * pBuffer, serial_manager_callback_message_t * message, serial_manager_status_t status)
{
    txCount--;
}
