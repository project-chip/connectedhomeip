/**
 ******************************************************************************
 * @file    hw_uart.c
 * @author  MCD Application Team
 * @brief   hardware access
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2018(-2021) STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "app_common.h"
#include "hw_conf.h"

/* Macros --------------------------------------------------------------------*/
#define HW_UART_INIT(__HANDLE__, __USART_BASE__)                                                                                   \
    do                                                                                                                             \
    {                                                                                                                              \
        (__HANDLE__).Instance                    = (__USART_BASE__);                                                               \
        (__HANDLE__).Init.BaudRate               = CFG_HW_##__USART_BASE__##_BAUDRATE;                                             \
        (__HANDLE__).Init.WordLength             = CFG_HW_##__USART_BASE__##_WORDLENGTH;                                           \
        (__HANDLE__).Init.StopBits               = CFG_HW_##__USART_BASE__##_STOPBITS;                                             \
        (__HANDLE__).Init.Parity                 = CFG_HW_##__USART_BASE__##_PARITY;                                               \
        (__HANDLE__).Init.HwFlowCtl              = CFG_HW_##__USART_BASE__##_HWFLOWCTL;                                            \
        (__HANDLE__).Init.Mode                   = CFG_HW_##__USART_BASE__##_MODE;                                                 \
        (__HANDLE__).Init.OverSampling           = CFG_HW_##__USART_BASE__##_OVERSAMPLING;                                         \
        (__HANDLE__).AdvancedInit.AdvFeatureInit = CFG_HW_##__USART_BASE__##_ADVFEATUREINIT;                                       \
        HAL_UART_Init(&(__HANDLE__));                                                                                              \
    } while (0)

#define HW_UART_RX_IT(__HANDLE__, __USART_BASE__)                                                                                  \
    do                                                                                                                             \
    {                                                                                                                              \
        HW_##__HANDLE__##RxCb = cb;                                                                                                \
        (__HANDLE__).Instance = (__USART_BASE__);                                                                                  \
        HAL_UART_Receive_IT(&(__HANDLE__), p_data, size);                                                                          \
    } while (0)

#define HW_UART_TX_IT(__HANDLE__, __USART_BASE__)                                                                                  \
    do                                                                                                                             \
    {                                                                                                                              \
        HW_##__HANDLE__##TxCb = cb;                                                                                                \
        (__HANDLE__).Instance = (__USART_BASE__);                                                                                  \
        HAL_UART_Transmit_IT(&(__HANDLE__), p_data, size);                                                                         \
    } while (0)

#define HW_UART_TX(__HANDLE__, __USART_BASE__)                                                                                     \
    do                                                                                                                             \
    {                                                                                                                              \
        (__HANDLE__).Instance = (__USART_BASE__);                                                                                  \
        hal_status            = HAL_UART_Transmit(&(__HANDLE__), p_data, size, timeout);                                           \
    } while (0)

#define HW_UART_MSP_UART_INIT(__HANDLE__, __USART_BASE__)                                                                          \
    do                                                                                                                             \
    {                                                                                                                              \
        /* Enable GPIO clock */                                                                                                    \
        CFG_HW_##__USART_BASE__##_TX_PORT_CLK_ENABLE();                                                                            \
                                                                                                                                   \
        /* Enable USART clock */                                                                                                   \
        __HAL_RCC_##__USART_BASE__##_CLK_ENABLE();                                                                                 \
                                                                                                                                   \
        GPIO_InitStruct.Pin       = CFG_HW_##__USART_BASE__##_TX_PIN;                                                              \
        GPIO_InitStruct.Mode      = CFG_HW_##__USART_BASE__##_TX_MODE;                                                             \
        GPIO_InitStruct.Pull      = CFG_HW_##__USART_BASE__##_TX_PULL;                                                             \
        GPIO_InitStruct.Speed     = CFG_HW_##__USART_BASE__##_TX_SPEED;                                                            \
        GPIO_InitStruct.Alternate = CFG_HW_##__USART_BASE__##_TX_ALTERNATE;                                                        \
                                                                                                                                   \
        HAL_GPIO_Init(CFG_HW_##__USART_BASE__##_TX_PORT, &GPIO_InitStruct);                                                        \
                                                                                                                                   \
        CFG_HW_##__USART_BASE__##_RX_PORT_CLK_ENABLE();                                                                            \
                                                                                                                                   \
        GPIO_InitStruct.Pin       = CFG_HW_##__USART_BASE__##_RX_PIN;                                                              \
        GPIO_InitStruct.Mode      = CFG_HW_##__USART_BASE__##_RX_MODE;                                                             \
        GPIO_InitStruct.Pull      = CFG_HW_##__USART_BASE__##_RX_PULL;                                                             \
        GPIO_InitStruct.Speed     = CFG_HW_##__USART_BASE__##_RX_SPEED;                                                            \
        GPIO_InitStruct.Alternate = CFG_HW_##__USART_BASE__##_RX_ALTERNATE;                                                        \
                                                                                                                                   \
        HAL_GPIO_Init(CFG_HW_##__USART_BASE__##_RX_PORT, &GPIO_InitStruct);                                                        \
                                                                                                                                   \
        CFG_HW_##__USART_BASE__##_CTS_PORT_CLK_ENABLE();                                                                           \
                                                                                                                                   \
        GPIO_InitStruct.Pin       = CFG_HW_##__USART_BASE__##_CTS_PIN;                                                             \
        GPIO_InitStruct.Mode      = CFG_HW_##__USART_BASE__##_CTS_MODE;                                                            \
        GPIO_InitStruct.Pull      = CFG_HW_##__USART_BASE__##_CTS_PULL;                                                            \
        GPIO_InitStruct.Speed     = CFG_HW_##__USART_BASE__##_CTS_SPEED;                                                           \
        GPIO_InitStruct.Alternate = CFG_HW_##__USART_BASE__##_CTS_ALTERNATE;                                                       \
                                                                                                                                   \
        HAL_GPIO_Init(CFG_HW_##__USART_BASE__##_CTS_PORT, &GPIO_InitStruct);                                                       \
                                                                                                                                   \
        HAL_NVIC_SetPriority(__USART_BASE__##_IRQn, CFG_HW_##__USART_BASE__##_PREEMPTPRIORITY,                                     \
                             CFG_HW_##__USART_BASE__##_SUBPRIORITY);                                                               \
        HAL_NVIC_EnableIRQ(__USART_BASE__##_IRQn);                                                                                 \
    } while (0)

#define HW_UART_MSP_TX_DMA_INIT(__HANDLE__, __USART_BASE__)                                                                        \
    do                                                                                                                             \
    {                                                                                                                              \
        /* Configure the DMA handler for Transmission process */                                                                   \
        /* Enable DMA clock */                                                                                                     \
        CFG_HW_##__USART_BASE__##_DMA_CLK_ENABLE();                                                                                \
        /* Enable DMA MUX clock */                                                                                                 \
        CFG_HW_##__USART_BASE__##_DMAMUX_CLK_ENABLE();                                                                             \
                                                                                                                                   \
        HW_hdma_##__HANDLE__##_tx.Instance                 = CFG_HW_##__USART_BASE__##_TX_DMA_CHANNEL;                             \
        HW_hdma_##__HANDLE__##_tx.Init.Request             = CFG_HW_##__USART_BASE__##_TX_DMA_REQ;                                 \
        HW_hdma_##__HANDLE__##_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;                                                 \
        HW_hdma_##__HANDLE__##_tx.Init.PeriphInc           = DMA_PINC_DISABLE;                                                     \
        HW_hdma_##__HANDLE__##_tx.Init.MemInc              = DMA_MINC_ENABLE;                                                      \
        HW_hdma_##__HANDLE__##_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;                                                  \
        HW_hdma_##__HANDLE__##_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;                                                  \
        HW_hdma_##__HANDLE__##_tx.Init.Mode                = DMA_NORMAL;                                                           \
        HW_hdma_##__HANDLE__##_tx.Init.Priority            = DMA_PRIORITY_LOW;                                                     \
                                                                                                                                   \
        HAL_DMA_Init(&HW_hdma_##__HANDLE__##_tx);                                                                                  \
                                                                                                                                   \
        /* Associate the initialized DMA handle to the UART handle */                                                              \
        __HAL_LINKDMA(huart, hdmatx, HW_hdma_##__HANDLE__##_tx);                                                                   \
                                                                                                                                   \
        /* NVIC configuration for DMA transfer complete interrupt  */                                                              \
        HAL_NVIC_SetPriority(CFG_HW_##__USART_BASE__##_TX_DMA_IRQn, CFG_HW_##__USART_BASE__##_DMA_TX_PREEMPTPRIORITY,              \
                             CFG_HW_##__USART_BASE__##_DMA_TX_SUBPRIORITY);                                                        \
        HAL_NVIC_EnableIRQ(CFG_HW_##__USART_BASE__##_TX_DMA_IRQn);                                                                 \
    } while (0)

/* Variables ------------------------------------------------------------------*/
#if (CFG_HW_USART1_ENABLED == 1)
UART_HandleTypeDef huart1 = { 0 };
#if (CFG_HW_USART1_DMA_TX_SUPPORTED == 1)
DMA_HandleTypeDef HW_hdma_huart1_tx = { 0 };
#endif
void (*HW_huart1RxCb)(void);
void (*HW_huart1TxCb)(void);
#endif

#if (CFG_HW_LPUART1_ENABLED == 1)
UART_HandleTypeDef lpuart1 = { 0 };
#if (CFG_HW_LPUART1_DMA_TX_SUPPORTED == 1)
DMA_HandleTypeDef HW_hdma_lpuart1_tx = { 0 };
#endif
void (*HW_lpuart1RxCb)(void);
void (*HW_lpuart1TxCb)(void);
#endif

void HW_UART_Init(hw_uart_id_t hw_uart_id)
{
    switch (hw_uart_id)
    {
#if (CFG_HW_USART1_ENABLED == 1)
    case hw_uart1:
        HW_UART_INIT(huart1, USART1);
        break;
#endif

#if (CFG_HW_LPUART1_ENABLED == 1)
    case hw_lpuart1:
        HW_UART_INIT(lpuart1, LPUART1);
        break;
#endif

    default:
        break;
    }

    return;
}

void HW_UART_Receive_IT(hw_uart_id_t hw_uart_id, uint8_t * p_data, uint16_t size, void (*cb)(void))
{
    switch (hw_uart_id)
    {
#if (CFG_HW_USART1_ENABLED == 1)
    case hw_uart1:
        HW_UART_RX_IT(huart1, USART1);
        break;
#endif

#if (CFG_HW_LPUART1_ENABLED == 1)
    case hw_lpuart1:
        HW_UART_RX_IT(lpuart1, LPUART1);
        break;
#endif

    default:
        break;
    }

    return;
}

void HW_UART_Transmit_IT(hw_uart_id_t hw_uart_id, uint8_t * p_data, uint16_t size, void (*cb)(void))
{
    switch (hw_uart_id)
    {
#if (CFG_HW_USART1_ENABLED == 1)
    case hw_uart1:
        HW_UART_TX_IT(huart1, USART1);
        break;
#endif

#if (CFG_HW_LPUART1_ENABLED == 1)
    case hw_lpuart1:
        HW_UART_TX_IT(lpuart1, LPUART1);
        break;
#endif

    default:
        break;
    }

    return;
}

hw_status_t HW_UART_Transmit(hw_uart_id_t hw_uart_id, uint8_t * p_data, uint16_t size, uint32_t timeout)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    hw_status_t hw_status        = hw_uart_ok;

    switch (hw_uart_id)
    {
#if (CFG_HW_USART1_ENABLED == 1)
    case hw_uart1:
        HW_UART_TX(huart1, USART1);
        break;
#endif

#if (CFG_HW_LPUART1_ENABLED == 1)
    case hw_lpuart1:
        HW_UART_TX(lpuart1, LPUART1);
        break;
#endif

    default:
        break;
    }

    switch (hal_status)
    {
    case HAL_OK:
        hw_status = hw_uart_ok;
        break;

    case HAL_ERROR:
        hw_status = hw_uart_error;
        break;

    case HAL_BUSY:
        hw_status = hw_uart_busy;
        break;

    case HAL_TIMEOUT:
        hw_status = hw_uart_to;
        break;

    default:
        break;
    }

    return hw_status;
}

hw_status_t HW_UART_Transmit_DMA(hw_uart_id_t hw_uart_id, uint8_t * p_data, uint16_t size, void (*cb)(void))
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    hw_status_t hw_status        = hw_uart_ok;

    switch (hw_uart_id)
    {
#if (CFG_HW_USART1_ENABLED == 1)
    case hw_uart1:
        HW_huart1TxCb   = cb;
        huart1.Instance = USART1;
        hal_status      = HAL_UART_Transmit_DMA(&huart1, p_data, size);
        break;
#endif

#if (CFG_HW_USART2_ENABLED == 1)
    case hw_uart2:
        HW_huart2TxCb   = cb;
        huart2.Instance = USART2;
        hal_status      = HAL_UART_Transmit_DMA(&huart2, p_data, size);
        break;
#endif

#if (CFG_HW_LPUART1_ENABLED == 1)
    case hw_lpuart1:
        HW_lpuart1TxCb   = cb;
        lpuart1.Instance = LPUART1;
        hal_status       = HAL_UART_Transmit_DMA(&lpuart1, p_data, size);
        break;
#endif

    default:
        break;
    }

    switch (hal_status)
    {
    case HAL_OK:
        hw_status = hw_uart_ok;
        break;

    case HAL_ERROR:
        hw_status = hw_uart_error;
        break;

    case HAL_BUSY:
        hw_status = hw_uart_busy;
        break;

    case HAL_TIMEOUT:
        hw_status = hw_uart_to;
        break;

    default:
        break;
    }

    return hw_status;
}

void HW_UART_Interrupt_Handler(hw_uart_id_t hw_uart_id)
{
    switch (hw_uart_id)
    {
#if (CFG_HW_USART1_ENABLED == 1)
    case hw_uart1:
        HAL_UART_IRQHandler(&huart1);
        break;
#endif

#if (CFG_HW_LPUART1_ENABLED == 1)
    case hw_lpuart1:
        HAL_UART_IRQHandler(&lpuart1);
        break;
#endif

    default:
        break;
    }

    return;
}

void HW_UART_DMA_Interrupt_Handler(hw_uart_id_t hw_uart_id)
{
    switch (hw_uart_id)
    {
#if (CFG_HW_USART1_DMA_TX_SUPPORTED == 1)
    case hw_uart1:
        HAL_DMA_IRQHandler(huart1.hdmatx);
        break;
#endif

#if (CFG_HW_USART2_DMA_TX_SUPPORTED == 1)
    case hw_uart2:
        HAL_DMA_IRQHandler(huart2.hdmatx);
        break;
#endif

#if (CFG_HW_LPUART1_DMA_TX_SUPPORTED == 1)
    case hw_lpuart1:
        HAL_DMA_IRQHandler(lpuart1.hdmatx);
        break;
#endif

    default:
        break;
    }

    return;
}

void HAL_UART_MspInit(UART_HandleTypeDef * huart)
{
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    switch ((uint32_t) huart->Instance)
    {
#if (CFG_HW_USART1_ENABLED == 1)
    case (uint32_t) USART1:
        HW_UART_MSP_UART_INIT(huart1, USART1);
#if (CFG_HW_USART1_DMA_TX_SUPPORTED == 1)
        HW_UART_MSP_TX_DMA_INIT(huart1, USART1);
#endif
        break;
#endif

#if (CFG_HW_LPUART1_ENABLED == 1)
    case (uint32_t) LPUART1:
        HW_UART_MSP_UART_INIT(lpuart1, LPUART1);
#if (CFG_HW_LPUART1_DMA_TX_SUPPORTED == 1)
        HW_UART_MSP_TX_DMA_INIT(lpuart1, LPUART1);
#endif
        break;
#endif

    default:
        break;
    }

    return;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart)
{
    switch ((uint32_t) huart->Instance)
    {
#if (CFG_HW_USART1_ENABLED == 1)
    case (uint32_t) USART1:
        if (HW_huart1RxCb)
        {
            HW_huart1RxCb();
        }
        break;
#endif

#if (CFG_HW_LPUART1_ENABLED == 1)
    case (uint32_t) LPUART1:
        if (HW_lpuart1RxCb)
        {
            HW_lpuart1RxCb();
        }
        break;
#endif

    default:
        break;
    }

    return;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef * huart)
{
    switch ((uint32_t) huart->Instance)
    {
#if (CFG_HW_USART1_ENABLED == 1)
    case (uint32_t) USART1:
        if (HW_huart1TxCb)
        {
            HW_huart1TxCb();
        }
        break;
#endif

#if (CFG_HW_LPUART1_ENABLED == 1)
    case (uint32_t) LPUART1:
        if (HW_lpuart1TxCb)
        {
            HW_lpuart1TxCb();
        }
        break;
#endif

    default:
        break;
    }

    return;
}
