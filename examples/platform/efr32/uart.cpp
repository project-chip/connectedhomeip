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
#include "AppConfig.h"
#include "matter_shell.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "assert.h"
#include "em_core.h"
#include "em_usart.h"
#include "sl_board_control.h"
#include "sl_uartdrv_instances.h"
#ifdef EFR32MG24
#include "sl_uartdrv_eusart_vcom_config.h"
#else
#include "sl_uartdrv_usart_vcom_config.h"
#endif // EFR32MG24
#include "uart.h"
#include "uartdrv.h"
#include <stddef.h>
#include <string.h>

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "sl_power_manager.h"
#endif

#if !defined(MIN)
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#endif

#ifdef EFR32MG24
#define HELPER1(x) EUSART##x##_RX_IRQn
#else
#define HELPER1(x) USART##x##_RX_IRQn
#endif

#define HELPER2(x) HELPER1(x)

#ifdef EFR32MG24
#define HELPER3(x) EUSART##x##_RX_IRQHandler
#else
#define HELPER3(x) USART##x##_RX_IRQHandler
#endif

#define HELPER4(x) HELPER3(x)

// On MG24 boards VCOM runs on the EUSART device, MG12 uses the UART device
#ifdef EFR32MG24
#define USART_IRQ HELPER2(SL_UARTDRV_EUSART_VCOM_PERIPHERAL_NO)
#define USART_IRQHandler HELPER4(SL_UARTDRV_EUSART_VCOM_PERIPHERAL_NO)
#define vcom_handle sl_uartdrv_eusart_vcom_handle
#else
#define USART_IRQ HELPER2(SL_UARTDRV_USART_VCOM_PERIPHERAL_NO)
#define USART_IRQHandler HELPER4(SL_UARTDRV_USART_VCOM_PERIPHERAL_NO)
#define vcom_handle sl_uartdrv_usart_vcom_handle
#endif // EFR32MG24

typedef struct
{
    // The data buffer
    uint8_t * pBuffer;
    // The offset of the first item written to the list.
    volatile uint16_t Head;
    // The offset of the next item to be written to the list.
    volatile uint16_t Tail;
    // Maxium size of data that can be hold in buffer before overwriting
    uint16_t MaxSize;
} Fifo_t;

#define UART_CONSOLE_ERR -1 // Negative value in case of UART Console action failed. Triggers a failure for PW_RPC
#define MAX_BUFFER_SIZE 256
#define MAX_DMA_BUFFER_SIZE (MAX_BUFFER_SIZE / 2)
// In order to reduce the probability of data loss during the dmaFull callback handler we use
// two duplicate receive buffers so we can always have one "active" receive queue.
static uint8_t sRxDmaBuffer[MAX_DMA_BUFFER_SIZE];
static uint8_t sRxDmaBuffer2[MAX_DMA_BUFFER_SIZE];
static uint16_t lastCount; // Nb of bytes already processed from the active dmaBuffer

// Rx buffer for the receive Fifo
static uint8_t sRxFifoBuffer[MAX_BUFFER_SIZE];
static Fifo_t sReceiveFifo;

static void UART_rx_callback(UARTDRV_Handle_t handle, Ecode_t transferStatus, uint8_t * data, UARTDRV_Count_t transferCount);

static bool InitFifo(Fifo_t * fifo, uint8_t * pDataBuffer, uint16_t bufferSize)
{
    if (fifo == NULL || pDataBuffer == NULL)
    {
        return false;
    }

    fifo->pBuffer = pDataBuffer;
    fifo->MaxSize = bufferSize;
    fifo->Tail = fifo->Head = 0;

    return true;
}

/*
 *   @brief Get the amount of unprocessed bytes in the fifo buffer
 *   @param Ptr to the fifo
 *   @return Nb of "unread" bytes available in the fifo
 */
static uint16_t AvailableDataCount(Fifo_t * fifo)
{
    uint16_t size = 0;

    // if equal there is no data return 0 directly
    if (fifo->Tail != fifo->Head)
    {
        // determine if a wrap around occurred to get the right data size avalaible.
        size = (fifo->Tail < fifo->Head) ? (fifo->MaxSize - fifo->Head + fifo->Tail) : (fifo->Tail - fifo->Head);
    }

    return size;
}

/*
 *   @brief Get the available space in the fifo buffer to insert new data
 *   @param Ptr to the fifo
 *   @return Nb of free bytes left in te buffer
 */
static uint16_t RemainingSpace(Fifo_t * fifo)
{
    return fifo->MaxSize - AvailableDataCount(fifo);
}

/*
 *   @brief Write data in the fifo as a circular buffer
 *   @param Ptr to the fifo, ptr of the data to write, nb of bytes to write
 */
static void WriteToFifo(Fifo_t * fifo, uint8_t * pDataToWrite, uint16_t SizeToWrite)
{
    assert(fifo);
    assert(pDataToWrite);
    assert(SizeToWrite <= fifo->MaxSize);

    // Overwrite is not allowed
    if (RemainingSpace(fifo) >= SizeToWrite)
    {
        uint16_t nBytesBeforWrap = (fifo->MaxSize - fifo->Tail);
        if (SizeToWrite > nBytesBeforWrap)
        {
            // The number of bytes to write is bigger than the remaining bytes
            // in the buffer, we have to wrap around
            memcpy(fifo->pBuffer + fifo->Tail, pDataToWrite, nBytesBeforWrap);
            memcpy(fifo->pBuffer, pDataToWrite + nBytesBeforWrap, SizeToWrite - nBytesBeforWrap);
        }
        else
        {
            memcpy(fifo->pBuffer + fifo->Tail, pDataToWrite, SizeToWrite);
        }

        fifo->Tail = (fifo->Tail + SizeToWrite) % fifo->MaxSize; // increment tail with wraparound
    }
}

/*
 *   @brief Write data in the fifo as a circular buffer
 *   @param Ptr to the fifo, ptr to contain the data to process, nb of bytes to pull from the fifo
 *   @return Nb of bytes that were retrieved.
 */
static uint8_t RetrieveFromFifo(Fifo_t * fifo, uint8_t * pData, uint16_t SizeToRead)
{
    assert(fifo);
    assert(pData);
    assert(SizeToRead <= fifo->MaxSize);

    uint16_t ReadSize        = MIN(SizeToRead, AvailableDataCount(fifo));
    uint16_t nBytesBeforWrap = (fifo->MaxSize - fifo->Head);

    if (ReadSize > nBytesBeforWrap)
    {
        memcpy(pData, fifo->pBuffer + fifo->Head, nBytesBeforWrap);
        memcpy(pData + nBytesBeforWrap, fifo->pBuffer, ReadSize - nBytesBeforWrap);
    }
    else
    {
        memcpy(pData, (fifo->pBuffer + fifo->Head), ReadSize);
    }

    fifo->Head = (fifo->Head + ReadSize) % fifo->MaxSize; // increment tail with wraparound

    return ReadSize;
}

/*
 *   @brief Init the the UART for serial communication, Start DMA reception
 *          and init Fifo to handle the received data from this uart
 *
 *   @Note This UART is used for pigweed rpc
 */
void uartConsoleInit(void)
{
    sl_board_enable_vcom();
    // Init a fifo for the data received on the uart
    InitFifo(&sReceiveFifo, sRxFifoBuffer, MAX_BUFFER_SIZE);

    // Activate 2 dma queues to always have one active

    UARTDRV_Receive(vcom_handle, sRxDmaBuffer, MAX_DMA_BUFFER_SIZE, UART_rx_callback);
    UARTDRV_Receive(vcom_handle, sRxDmaBuffer2, MAX_DMA_BUFFER_SIZE, UART_rx_callback);

    // Enable USART0/EUSART0 interrupt to wake OT task when data arrives
    NVIC_ClearPendingIRQ(USART_IRQ);
    NVIC_EnableIRQ(USART_IRQ);

#ifdef EFR32MG24
    // Clear previous RX interrupts
    EUSART_IntClear(SL_UARTDRV_EUSART_VCOM_PERIPHERAL, EUSART_IF_RXFL);

    // Enable RX interrupts
    EUSART_IntEnable(SL_UARTDRV_EUSART_VCOM_PERIPHERAL, EUSART_IF_RXFL);

    // Enable EUSART
    EUSART_Enable(SL_UARTDRV_EUSART_VCOM_PERIPHERAL, eusartEnable);
#else
    USART_IntEnable(SL_UARTDRV_USART_VCOM_PERIPHERAL, USART_IF_RXDATAV);
#endif // EFR32MG24
}

void USART_IRQHandler(void)
{
#ifdef ENABLE_CHIP_SHELL
    chip::NotifyShellProcessFromISR();
#endif
#if defined(SL_WIFI)
    /* TODO */
#elif !defined(PW_RPC_ENABLED)
    otSysEventSignalPending();
#endif

#ifdef EFR32MG24
    EUSART_IntClear(SL_UARTDRV_EUSART_VCOM_PERIPHERAL, EUSART_IF_RXFL);
#endif
}

/*
 *   @brief Callback triggered when a UARTDRV DMA buffer is full
 */
static void UART_rx_callback(UARTDRV_Handle_t handle, Ecode_t transferStatus, uint8_t * data, UARTDRV_Count_t transferCount)
{
    (void) transferStatus;

    uint8_t writeSize = (transferCount - lastCount);
    if (RemainingSpace(&sReceiveFifo) >= writeSize)
    {
        WriteToFifo(&sReceiveFifo, data + lastCount, writeSize);
        lastCount = 0;
    }

    UARTDRV_Receive(vcom_handle, data, transferCount, UART_rx_callback);

#ifdef ENABLE_CHIP_SHELL
    chip::NotifyShellProcessFromISR();
#endif
#if defined(SL_WIFI)
    /* TODO */
#elif !defined(PW_RPC_ENABLED)
    otSysEventSignalPending();
#endif
}

/*
 *   @brief Read the data available from the console Uart
 *   @param Buffer that contains the data to write, number bytes to write.
 *   @return Amount of bytes written or ERROR (-1)
 */
int16_t uartConsoleWrite(const char * Buf, uint16_t BufLength)
{
    if (Buf == NULL || BufLength < 1)
    {
        return UART_CONSOLE_ERR;
    }

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
    sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
#endif

    // Use of ForceTransmit here. Transmit with DMA was causing errors with PW_RPC
    // TODO Use DMA and find/fix what causes the issue with PW
    if (UARTDRV_ForceTransmit(vcom_handle, (uint8_t *) Buf, BufLength) == ECODE_EMDRV_UARTDRV_OK)
    {
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
        sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
#endif
        return BufLength;
    }

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
    sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
#endif

    return UART_CONSOLE_ERR;
}

/*
 *   @brief Read the data available from the console Uart
 *   @param Buffer for the data to be read, number bytes to read.
 *   @return Amount of bytes that was read from the rx fifo or ERROR (-1)
 */
int16_t uartConsoleRead(char * Buf, uint16_t NbBytesToRead)
{
    uint8_t * data;
    UARTDRV_Count_t count, remaining;

    if (Buf == NULL || NbBytesToRead < 1)
    {
        return UART_CONSOLE_ERR;
    }

    if (NbBytesToRead > AvailableDataCount(&sReceiveFifo))
    {
        // Not enough data available in the fifo for the read size request
        // If there is data available in dma buffer, get it now.
        CORE_ATOMIC_SECTION(UARTDRV_GetReceiveStatus(vcom_handle, &data, &count, &remaining); if (count > lastCount) {
            WriteToFifo(&sReceiveFifo, data + lastCount, count - lastCount);
            lastCount = count;
        })
    }

    return (int16_t) RetrieveFromFifo(&sReceiveFifo, (uint8_t *) Buf, NbBytesToRead);
}

#ifdef __cplusplus
}
#endif
