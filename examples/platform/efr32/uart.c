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
#include "AppConfig.h"
#include "assert.h"
#include "em_core.h"
#include "em_usart.h"
#include "hal-config.h"
#include "uartdrv.h"
#include <stddef.h>
#include <string.h>

#if !defined(MIN)
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#endif

DEFINE_BUF_QUEUE(EMDRV_UARTDRV_MAX_CONCURRENT_RX_BUFS, sUartRxQueue);
DEFINE_BUF_QUEUE(EMDRV_UARTDRV_MAX_CONCURRENT_TX_BUFS, sUartTxQueue);

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

static UARTDRV_HandleData_t sUartHandleData;
static UARTDRV_Handle_t sUartHandle = &sUartHandleData;

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
        // determine if a wrap around occured to get the right data size avalaible.
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
    UARTDRV_Init_t uartInit = {
        .port     = USART0,
        .baudRate = HAL_SERIAL_APP_BAUD_RATE,
#if defined(_USART_ROUTELOC0_MASK)
        .portLocationTx = BSP_SERIAL_APP_TX_LOC,
        .portLocationRx = BSP_SERIAL_APP_RX_LOC,
#elif defined(_USART_ROUTE_MASK)
#error This configuration is not supported
#elif defined(_GPIO_USART_ROUTEEN_MASK)
        .txPort  = BSP_SERIAL_APP_TX_PORT, /* USART Tx port number */
        .rxPort  = BSP_SERIAL_APP_RX_PORT, /* USART Rx port number */
        .txPin   = BSP_SERIAL_APP_TX_PIN,  /* USART Tx pin number */
        .rxPin   = BSP_SERIAL_APP_RX_PIN,  /* USART Rx pin number */
        .uartNum = 0,                      /* UART instance number */
#endif
#if defined(USART_CTRL_MVDIS)
        .mvdis = false,
#endif
        .stopBits     = (USART_Stopbits_TypeDef) USART_FRAME_STOPBITS_ONE,
        .parity       = (USART_Parity_TypeDef) USART_FRAME_PARITY_NONE,
        .oversampling = (USART_OVS_TypeDef) USART_CTRL_OVS_X16,
        .fcType       = HAL_SERIAL_APP_FLOW_CONTROL,
        .ctsPort      = BSP_SERIAL_APP_CTS_PORT,
        .ctsPin       = BSP_SERIAL_APP_CTS_PIN,
        .rtsPort      = BSP_SERIAL_APP_RTS_PORT,
        .rtsPin       = BSP_SERIAL_APP_RTS_PIN,
        .rxQueue      = (UARTDRV_Buffer_FifoQueue_t *) &sUartRxQueue,
        .txQueue      = (UARTDRV_Buffer_FifoQueue_t *) &sUartTxQueue,
#if defined(_USART_ROUTELOC1_MASK)
        .portLocationCts = BSP_SERIAL_APP_CTS_LOC,
        .portLocationRts = BSP_SERIAL_APP_RTS_LOC,
#endif
    };

    // Init a fifo for the data received on the uart
    InitFifo(&sReceiveFifo, sRxFifoBuffer, MAX_BUFFER_SIZE);

    UARTDRV_InitUart(sUartHandle, &uartInit);
    // Activate 2 dma queues to always have one active
    UARTDRV_Receive(sUartHandle, sRxDmaBuffer, MAX_DMA_BUFFER_SIZE, UART_rx_callback);
    UARTDRV_Receive(sUartHandle, sRxDmaBuffer2, MAX_DMA_BUFFER_SIZE, UART_rx_callback);
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

    UARTDRV_Receive(sUartHandle, data, transferCount, UART_rx_callback);
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

    // Use of ForceTransmit here. Transmit with DMA was causing errors with PW_RPC
    // TODO Use DMA and find/fix what causes the issue with PW
    if (UARTDRV_ForceTransmit(sUartHandle, (uint8_t *) Buf, BufLength) == ECODE_EMDRV_UARTDRV_OK)
    {
        return BufLength;
    }

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
        CORE_ATOMIC_SECTION(UARTDRV_GetReceiveStatus(sUartHandle, &data, &count, &remaining); if (count > lastCount) {
            WriteToFifo(&sReceiveFifo, data + lastCount, count - lastCount);
            lastCount = count;
        })
    }

    return (int16_t) RetrieveFromFifo(&sReceiveFifo, (uint8_t *) Buf, NbBytesToRead);
}
