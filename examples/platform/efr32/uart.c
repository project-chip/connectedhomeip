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
#include "uartdrv.h"
#include <stddef.h>
#include "assert.h"
#include "AppConfig.h"
#include <string.h>

#if !defined(min)
#define min(A,B) ( (A) < (B) ? (A):(B))
#endif

DEFINE_BUF_QUEUE(EMDRV_UARTDRV_MAX_CONCURRENT_RX_BUFS, sUartRxQueue);
DEFINE_BUF_QUEUE(EMDRV_UARTDRV_MAX_CONCURRENT_TX_BUFS, sUartTxQueue);

static UARTDRV_HandleData_t sUartHandleData;
static UARTDRV_Handle_t sUartHandle = &sUartHandleData;

typedef struct ReceiveFifo_t
{
    // The data buffer
    uint8_t *pBuffer;
    // The offset of the first item written to the list.
    volatile uint16_t Head;
    // The offset of the next item to be written to the list.
    volatile uint16_t Tail;
    // Maxium size of data that can be hold in buffer before overwriting
    uint16_t MaxSize;
} ReceiveFifo_t;

#define MAX_BUFFER_SIZE 128
// In order to reduce the probability of data loss during the dmaFull callback handler we use
// two duplicate receive buffers so we can always have one "active" receive queue.
static uint8_t sRxDmaBuffer[MAX_BUFFER_SIZE];
static uint8_t sRxDmaBuffer2[MAX_BUFFER_SIZE];
static uint8_t sRxFifoBuffer[MAX_BUFFER_SIZE];
static uint16_t lastCount;
static ReceiveFifo_t sReceiveFifo;

//static void UART_tx_callback(UARTDRV_Handle_t handle, Ecode_t transferStatus, uint8_t *data, UARTDRV_Count_t transferCount);
static void UART_rx_callback(UARTDRV_Handle_t handle, Ecode_t transferStatus, uint8_t *data, UARTDRV_Count_t transferCount);

static bool InitFifo(ReceiveFifo_t *fifo, uint8_t* pDataBuffer, uint16_t bufferSize)
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

static uint16_t AvailableDataCount(ReceiveFifo_t *fifo)
{
    uint16_t size = 0;
    
    // if equal there is no data return 0 directly
    if (fifo->Tail != fifo->Head)
    {
        // determine if a wrap around occured to get the right data size avalaible.
        size = (fifo->Tail < fifo->Head) ? (fifo->MaxSize - fifo->Head + fifo->Tail) : (fifo->Tail - fifo->Head) ;
    }

    return size;
}

static uint16_t RemainingSpace(ReceiveFifo_t *fifo)
{
    return fifo->MaxSize - AvailableDataCount(fifo);
}

static void WriteToFifo(ReceiveFifo_t *fifo, uint8_t* pDataToWrite, uint16_t SizeToWrite)
{
    assert(fifo);
    assert(pDataToWrite);
    assert(SizeToWrite <= fifo->MaxSize);

    // Overwrite is not allowed
    if ( RemainingSpace(fifo) >= SizeToWrite )
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

static uint8_t RetrieveFromFifo(ReceiveFifo_t *fifo, uint8_t* pData, uint16_t SizeToRead)
{
    assert(fifo);
    assert(pData);
    assert(SizeToRead <= fifo->MaxSize);

    uint16_t ReadSize =  min(SizeToRead, AvailableDataCount(fifo));
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

void uartConsoleInit(void)
{
    UARTDRV_Init_t uartInit = {
        .port     = USART0,
        .baudRate = HAL_SERIAL_APP_BAUD_RATE,
#if defined(_USART_ROUTELOC0_MASK)
        .portLocationTx = BSP_SERIAL_APP_TX_LOC,
        .portLocationRx = BSP_SERIAL_APP_RX_LOC,
#elif defined(_USART_ROUTE_MASK)
        portLocation = BSP_SERIAL_APP_PORT,
#endif
#if defined(USART_CTRL_MVDIS)
        .mvdis = false,
#endif
        .stopBits     = (USART_Stopbits_TypeDef) USART_FRAME_STOPBITS_ONE,
        .parity       = (USART_Parity_TypeDef) USART_FRAME_PARITY_NONE,
        .oversampling = (USART_OVS_TypeDef) USART_CTRL_OVS_X16,
        .fcType  = HAL_SERIAL_APP_FLOW_CONTROL,
        .ctsPort = BSP_SERIAL_APP_CTS_PORT,
        .ctsPin  = BSP_SERIAL_APP_CTS_PIN,
        .rtsPort = BSP_SERIAL_APP_RTS_PORT,
        .rtsPin  = BSP_SERIAL_APP_RTS_PIN,
        .rxQueue = (UARTDRV_Buffer_FifoQueue_t *) &sUartRxQueue,
        .txQueue = (UARTDRV_Buffer_FifoQueue_t *) &sUartTxQueue,
#if defined(_USART_ROUTELOC1_MASK)
        .portLocationCts = BSP_SERIAL_APP_CTS_LOC,
        .portLocationRts = BSP_SERIAL_APP_RTS_LOC,
#endif
    };

    InitFifo(&sReceiveFifo , sRxFifoBuffer, MAX_BUFFER_SIZE);

    UARTDRV_InitUart(sUartHandle, &uartInit);
    UARTDRV_Receive(sUartHandle, sRxDmaBuffer, MAX_BUFFER_SIZE, UART_rx_callback);
    UARTDRV_Receive(sUartHandle, sRxDmaBuffer2, MAX_BUFFER_SIZE, UART_rx_callback);
}

// static void UART_tx_callback(UARTDRV_Handle_t handle, Ecode_t transferStatus, uint8_t *data, UARTDRV_Count_t transferCount)
// {
//     (void)handle;
//     (void)transferStatus;
//     (void)data;
//     (void)transferCount;
// }

// Callback triggered when UARTDRV DMA has received data
static void UART_rx_callback(UARTDRV_Handle_t handle,
                             Ecode_t transferStatus,
                             uint8_t *data,
                             UARTDRV_Count_t transferCount)
{
    (void)transferStatus;

    uint8_t writeSize = (transferCount-lastCount);
    if ( RemainingSpace(&sReceiveFifo) >= writeSize )
    {
        WriteToFifo(&sReceiveFifo, data+lastCount, writeSize);
        lastCount = 0;
    }

    UARTDRV_Receive(sUartHandle, data, transferCount, UART_rx_callback);
}

int16_t uartConsoleWrite(const char * Buf, uint16_t BufLength)
{
    if (Buf == NULL || BufLength < 1)
    {
        return -1;
    }

    if (UARTDRV_ForceTransmit(sUartHandle, (uint8_t *) Buf, BufLength) == ECODE_EMDRV_UARTDRV_OK)
    {
        return BufLength;
    }
    
    return -1;
}

int16_t uartConsoleRead(char * Buf, uint16_t BufLength)
{
    uint8_t *       data;
    UARTDRV_Count_t count, remaining;
    
    if (Buf == NULL || BufLength < 1)
    {
        return -1;
    }

    if (BufLength > AvailableDataCount(&sReceiveFifo) )
    {
        // If there is data available in dma buffer, get it now.
        CORE_ATOMIC_SECTION(UARTDRV_GetReceiveStatus(sUartHandle, &data, &count, &remaining);
            if (count > lastCount) 
            {
                WriteToFifo(&sReceiveFifo, data + lastCount, count - lastCount);
                lastCount = count;
            })
    }

    return (int16_t)RetrieveFromFifo(&sReceiveFifo, (uint8_t*)Buf, BufLength);
}
