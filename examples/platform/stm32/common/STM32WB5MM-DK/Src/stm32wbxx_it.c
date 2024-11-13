/**
 ******************************************************************************
 * @file    stm32wbxx_it.c
 * @author  MCD Application Team
 * @brief   Main Interrupt Service Routines.
 *          This file provides template for all exceptions handler and
 *          peripherals interrupt service routine.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2019-2021 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32wbxx_it.h"
#include "app_common.h"

/* CONCURRENT MODE BLE/THREAD */
/* External variables  -----------------------------------------------------------*/
extern uint8_t ThreadEnable;
extern TIM_HandleTypeDef htim17;

/* /THREAD */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
 * @brief  This function handles NMI exception.
 * @param  None
 * @retval None
 */
void NMI_Handler(void) {}

/**
 * @brief  This function handles Hard Fault exception.
 * @param  None
 * @retval None
 */
void HardFault_Handler(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1)
    {
    }
}

/**
 * @brief This function handles Memory management fault.
 */
void MemManage_Handler(void)
{
    /* USER CODE BEGIN MemoryManagement_IRQn 0 */

    /* USER CODE END MemoryManagement_IRQn 0 */
    while (1)
    {
        /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
        /* USER CODE END W1_MemoryManagement_IRQn 0 */
    }
}

/**
 * @brief  This function handles Debug Monitor exception.
 * @param  None
 * @retval None
 */
void DebugMon_Handler(void) {}

void IPCC_C1_TX_IRQHandler(void)
{
    HW_IPCC_Tx_Handler();

    return;
}

void IPCC_C1_RX_IRQHandler(void)
{
    HW_IPCC_Rx_Handler();
    return;
}

/**
 * @brief This function handles TIM1 trigger and commutation interrupts and TIM17 global interrupt.
 */
void TIM1_TRG_COM_TIM17_IRQHandler(void)
{
    /* USER CODE BEGIN TIM1_TRG_COM_TIM17_IRQn 0 */
    /* USER CODE END TIM1_TRG_COM_TIM17_IRQn 0 */

    HAL_TIM_IRQHandler(&htim17);

    /* USER CODE BEGIN TIM1_TRG_COM_TIM17_IRQn 1 */
    /* USER CODE END TIM1_TRG_COM_TIM17_IRQn 1 */
}

/**
 * @brief  This function handles External line
 *         interrupt request.
 * @param  None
 * @retval None
 */
void EXTI15_10_IRQHandler(void)
{
    BSP_PB_IRQHandler(BUTTON_USER1);
}

#if (CFG_HW_USART1_ENABLED == 1)
void USART1_IRQHandler(void)
{
    HW_UART_Interrupt_Handler(hw_uart1);
}
#endif

#if (CFG_HW_USART1_DMA_TX_SUPPORTED == 1)
void CFG_HW_USART1_DMA_TX_IRQHandler(void)
{
    HW_UART_DMA_Interrupt_Handler(hw_uart1);
}
#endif

#if (CFG_HW_LPUART1_ENABLED == 1)
void LPUART1_IRQHandler(void)
{
    HW_UART_Interrupt_Handler(hw_lpuart1);
}
#endif

#if (CFG_HW_LPUART1_DMA_TX_SUPPORTED == 1)
void CFG_HW_LPUART1_DMA_TX_IRQHandler(void)
{
    HW_UART_DMA_Interrupt_Handler(hw_lpuart1);
}
#endif

/******************************************************************************/
/*                 STM32L0xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32l0xx.s).                                               */
/******************************************************************************/
/**
 * @brief  This function handles RTC Auto wake-up interrupt request.
 * @param  None
 * @retval None
 */
void RTC_WKUP_IRQHandler(void)
{
    HW_TS_RTC_Wakeup_Handler();
}
