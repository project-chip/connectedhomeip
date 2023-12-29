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
#include "FreeRTOS.h"
#include "event_groups.h"
#include "matter_shell.h"
#include "semphr.h"
#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "assert.h"
#include "em_core.h"
#include "em_usart.h"
#ifdef SL_BOARD_NAME
#include "sl_board_control.h"
#endif
#include "sl_uartdrv_instances.h"
#if SL_WIFI
#include "spi_multiplex.h"
#endif // SL_WIFI
#ifdef SL_CATALOG_UARTDRV_EUSART_PRESENT
#include "sl_uartdrv_eusart_vcom_config.h"
#endif
#ifdef SL_CATALOG_UARTDRV_USART_PRESENT
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

#ifdef SL_CATALOG_UARTDRV_EUSART_PRESENT
#define HELPER1(x) EUSART##x##_RX_IRQn
#else
#define HELPER1(x) USART##x##_RX_IRQn
#endif

#define HELPER2(x) HELPER1(x)

#ifdef SL_CATALOG_UARTDRV_EUSART_PRESENT
#define HELPER3(x) EUSART##x##_RX_IRQHandler
#else
#define HELPER3(x) USART##x##_RX_IRQHandler
#endif

#define HELPER4(x) HELPER3(x)

// On MG24 boards VCOM runs on the EUSART device, MG12 uses the UART device
#ifdef SL_CATALOG_UARTDRV_EUSART_PRESENT
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

// uart transmit
#if SILABS_LOG_OUT_UART
#define UART_MAX_QUEUE_SIZE 125
#else
#define UART_MAX_QUEUE_SIZE 25
#endif
#define UART_TASK_SIZE 256
#define UART_TASK_NAME "UART"

#ifdef CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE
#define UART_TX_MAX_BUF_LEN (CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE + 2) // \r\n
#else
#define UART_TX_MAX_BUF_LEN (258)
#endif

static TaskHandle_t sUartTaskHandle;
static StackType_t uartStack[UART_TASK_SIZE * sizeof(StackType_t)];
static StaticTask_t uartTaskStruct;

typedef struct
{
    uint8_t data[UART_TX_MAX_BUF_LEN];
    uint16_t length = 0;
} UartTxStruct_t;

uint8_t sUartTxQueueBuffer[UART_MAX_QUEUE_SIZE * sizeof(UartTxStruct_t)];
static StaticQueue_t sUartTxQueueStruct;
static QueueHandle_t sUartTxQueue;

// Rx buffer for the receive Fifo
static uint8_t sRxFifoBuffer[MAX_BUFFER_SIZE];
static Fifo_t sReceiveFifo;

static void UART_rx_callback(UARTDRV_Handle_t handle, Ecode_t transferStatus, uint8_t * data, UARTDRV_Count_t transferCount);
static void uartSendBytes(uint8_t * buffer, uint16_t nbOfBytes);

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
static uint16_t RetrieveFromFifo(Fifo_t * fifo, uint8_t * pData, uint16_t SizeToRead)
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
    if (sUartTaskHandle != NULL)
    {
        // Init was already done
        return;
    }

#ifdef SL_BOARD_NAME
    sl_board_enable_vcom();
#endif
    // Init a fifo for the data received on the uart
    InitFifo(&sReceiveFifo, sRxFifoBuffer, MAX_BUFFER_SIZE);

    // Activate 2 dma queues to always have one active
    UARTDRV_Receive(vcom_handle, sRxDmaBuffer, MAX_DMA_BUFFER_SIZE, UART_rx_callback);
    UARTDRV_Receive(vcom_handle, sRxDmaBuffer2, MAX_DMA_BUFFER_SIZE, UART_rx_callback);

    sUartTxQueue    = xQueueCreateStatic(UART_MAX_QUEUE_SIZE, sizeof(UartTxStruct_t), sUartTxQueueBuffer, &sUartTxQueueStruct);
    sUartTaskHandle = xTaskCreateStatic(uartMainLoop, UART_TASK_NAME, UART_TASK_SIZE, nullptr, 30, uartStack, &uartTaskStruct);

    assert(sUartTaskHandle);
    assert(sUartTxQueue);

    // Enable USART0/EUSART0 interrupt to wake OT task when data arrives
    NVIC_ClearPendingIRQ(USART_IRQ);
    NVIC_EnableIRQ(USART_IRQ);

#ifdef SL_CATALOG_UARTDRV_EUSART_PRESENT
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
#elif !defined(PW_RPC_ENABLED)
    otSysEventSignalPending();
#endif

#ifdef SL_CATALOG_UARTDRV_EUSART_PRESENT
    EUSART_IntClear(SL_UARTDRV_EUSART_VCOM_PERIPHERAL, EUSART_IF_RXFL);
#endif
}

/**
 * @brief Transmit complete callback
 *
 * @param handle
 * @param transferStatus
 * @param data
 * @param transferCount
 */
void UART_tx_callback(struct UARTDRV_HandleData * handle, Ecode_t transferStatus, uint8_t * data, UARTDRV_Count_t transferCount)
{
    BaseType_t xHigherPriorityTaskWoken;

    vTaskNotifyGiveFromISR(sUartTaskHandle, &xHigherPriorityTaskWoken) portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
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
#elif !defined(PW_RPC_ENABLED)
    otSysEventSignalPending();
#endif
}

/**
 * @brief Read the data available from the console Uart
 *
 * @param Buf Buffer that contains the data to write
 * @param BufLength number bytes to write
 * @return int16_t Amount of bytes written or ERROR (-1)
 */
int16_t uartConsoleWrite(const char * Buf, uint16_t BufLength)
{
    if (Buf == NULL || BufLength < 1 || BufLength > UART_TX_MAX_BUF_LEN)
    {
        return UART_CONSOLE_ERR;
    }

#ifdef PW_RPC_ENABLED
    // Pigweed Logger is already thread safe.
    UARTDRV_ForceTransmit(vcom_handle, (uint8_t *) Buf, BufLength);
    return BufLength;
#endif

    UartTxStruct_t workBuffer;
    memcpy(workBuffer.data, Buf, BufLength);
    workBuffer.length = BufLength;

    if (xPortIsInsideInterrupt())
    {
        BaseType_t xHigherPriorityTaskWoken;
        xQueueSendFromISR(sUartTxQueue, &workBuffer, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        return BufLength;
    }
    else
    {
        if (pdTRUE == xQueueSend(sUartTxQueue, &workBuffer, portMAX_DELAY))
        {
            return BufLength;
        }
    }

    return UART_CONSOLE_ERR;
}

/**
 * @brief Write Logs to the Uart. Appends a return character
 *
 * @param log pointer to the logs
 * @param length number of bytes to write
 * @return int16_t Amount of bytes written or ERROR (-1)
 */
int16_t uartLogWrite(const char * log, uint16_t length)
{
    if (log == NULL || length < 1 || (length + 2) > UART_TX_MAX_BUF_LEN)
    {
        return UART_CONSOLE_ERR;
    }

    UartTxStruct_t workBuffer;
    memcpy(workBuffer.data, log, length);
    memcpy(workBuffer.data + length, "\r\n", 2);
    workBuffer.length = length + 2;

    if (xPortIsInsideInterrupt())
    {
        BaseType_t xHigherPriorityTaskWoken;
        xQueueSendFromISR(sUartTxQueue, &workBuffer, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        return length;
    }
    else
    {
        if (pdTRUE == xQueueSend(sUartTxQueue, &workBuffer, 0))
        {
            return length;
        }
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
        CORE_ATOMIC_SECTION(UARTDRV_GetReceiveStatus(vcom_handle, &data, &count, &remaining); if (count > lastCount) {
            WriteToFifo(&sReceiveFifo, data + lastCount, count - lastCount);
            lastCount = count;
        })
    }

    return (int16_t) RetrieveFromFifo(&sReceiveFifo, (uint8_t *) Buf, NbBytesToRead);
}

void uartMainLoop(void * args)
{
    UartTxStruct_t workBuffer;

    while (1)
    {

        BaseType_t eventReceived = xQueueReceive(sUartTxQueue, &workBuffer, portMAX_DELAY);
        while (eventReceived == pdTRUE)
        {
            uartSendBytes(workBuffer.data, workBuffer.length);
            eventReceived = xQueueReceive(sUartTxQueue, &workBuffer, 0);
        }
    }
}

/**
 * @brief Send Bytes to UART. This blocks the UART task.
 *
 * @param buffer pointer to the buffer containing the data
 * @param nbOfBytes number of bytes to send
 */
void uartSendBytes(uint8_t * buffer, uint16_t nbOfBytes)
{

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
    sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
#endif
#if SL_UARTCTRL_MUX
    sl_wfx_host_pre_uart_transfer();
#endif // SL_UARTCTRL_MUX
#if (defined(EFR32MG24) && defined(WF200_WIFI))
    // Blocking transmit for the MG24 + WF200 since UART TX is multiplexed with
    // WF200 SPI IRQ
    UARTDRV_ForceTransmit(vcom_handle, (uint8_t *) buffer, nbOfBytes);
#else
    // Non Blocking Transmit
    UARTDRV_Transmit(vcom_handle, (uint8_t *) buffer, nbOfBytes, UART_tx_callback);
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
#endif /* EFR32MG24 && WF200_WIFI */
#if SL_UARTCTRL_MUX
    sl_wfx_host_post_uart_transfer();
#endif // SL_UARTCTRL_MUX

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
    sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
#endif
}

#ifdef __cplusplus
}
#endif
