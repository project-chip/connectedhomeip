/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    stm32wbaxx_it.c
 * @brief   Interrupt Service Routines.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "stm32wbaxx_it.h"
#include "app_conf.h"
#include "ll_sys.h"
#include "main.h"
#include "scm.h"
#include "stm32wbaxx_hal.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_bsp.h"
#include "pka_ctrl.h"
/* USER CODE END Includes */

/* External functions --------------------------------------------------------*/
extern void (*radio_callback)(void);
extern void (*low_isr_callback)(void);

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern volatile uint8_t radio_sw_low_isr_is_running_high_prio;
extern RTC_HandleTypeDef hrtc;
extern DMA_HandleTypeDef handle_GPDMA1_Channel1;
extern DMA_HandleTypeDef handle_GPDMA1_Channel0;
extern DMA_HandleTypeDef handle_GPDMA1_Channel3;
extern DMA_HandleTypeDef handle_GPDMA1_Channel2;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
 * @brief This function handles Non maskable interrupt.
 */
void NMI_Handler(void)
{
    /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

    /* USER CODE END NonMaskableInt_IRQn 0 */
    /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
    while (1)
    {
    }
    /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
 * @brief This function handles Hard fault interrupt.
 */
void HardFault_Handler(void)
{
    /* USER CODE BEGIN HardFault_IRQn 0 */

    /* USER CODE END HardFault_IRQn 0 */
    while (1)
    {
        /* USER CODE BEGIN W1_HardFault_IRQn 0 */
        /* USER CODE END W1_HardFault_IRQn 0 */
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
 * @brief This function handles Prefetch fault, memory access fault.
 */
void BusFault_Handler(void)
{
    /* USER CODE BEGIN BusFault_IRQn 0 */

    /* USER CODE END BusFault_IRQn 0 */
    while (1)
    {
        /* USER CODE BEGIN W1_BusFault_IRQn 0 */
        /* USER CODE END W1_BusFault_IRQn 0 */
    }
}

/**
 * @brief This function handles Undefined instruction or illegal state.
 */
void UsageFault_Handler(void)
{
    /* USER CODE BEGIN UsageFault_IRQn 0 */

    /* USER CODE END UsageFault_IRQn 0 */
    while (1)
    {
        /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
        /* USER CODE END W1_UsageFault_IRQn 0 */
    }
}

/**
 * @brief This function handles Debug monitor.
 */
void DebugMon_Handler(void)
{
    /* USER CODE BEGIN DebugMonitor_IRQn 0 */

    /* USER CODE END DebugMonitor_IRQn 0 */
    /* USER CODE BEGIN DebugMonitor_IRQn 1 */

    /* USER CODE END DebugMonitor_IRQn 1 */
}

/******************************************************************************/
/* STM32WBAxx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32wbaxx.s).                    */
/******************************************************************************/

/**
 * @brief This function handles RTC non-secure interrupt.
 */
void RTC_IRQHandler(void)
{
    /* USER CODE BEGIN RTC_IRQn 0 */

    /* USER CODE END RTC_IRQn 0 */
    HAL_RTC_AlarmIRQHandler(&hrtc);
    HAL_RTCEx_SSRUIRQHandler(&hrtc);
    /* USER CODE BEGIN RTC_IRQn 1 */

    /* USER CODE END RTC_IRQn 1 */
}

/**
 * @brief This function handles RCC non-secure global interrupt.
 */
void RCC_IRQHandler(void)
{
    /* USER CODE BEGIN RCC_IRQn 0 */

    /* USER CODE END RCC_IRQn 0 */
    /* Check the RCC interrupt source */
    if (__HAL_RCC_GET_IT(RCC_IT_HSERDY))
    {
        __HAL_RCC_CLEAR_IT(RCC_IT_HSERDY);
#if (CFG_SCM_SUPPORTED == 1)
        /* SCM HSE BEGIN */
        SCM_HSE_StartStabilizationTimer();
        /* SCM HSE END */
#endif /* CFG_SCM_SUPPORTED */
    }
    else if (__HAL_RCC_GET_IT(RCC_IT_PLL1RDY))
    {
        __HAL_RCC_CLEAR_IT(RCC_IT_PLL1RDY);
#if (CFG_SCM_SUPPORTED == 1)
        scm_pllrdy_isr();
#endif /* CFG_SCM_SUPPORTED */
    }
    /* USER CODE BEGIN RCC_IRQn 1 */

    /* USER CODE END RCC_IRQn 1 */
}

/**
 * @brief This function handles GPDMA1 Channel 0 global interrupt.
 */
void GPDMA1_Channel0_IRQHandler(void)
{
    /* USER CODE BEGIN GPDMA1_Channel0_IRQn 0 */

    /* USER CODE END GPDMA1_Channel0_IRQn 0 */
    HAL_DMA_IRQHandler(&handle_GPDMA1_Channel0);
    /* USER CODE BEGIN GPDMA1_Channel0_IRQn 1 */

    /* USER CODE END GPDMA1_Channel0_IRQn 1 */
}

/**
 * @brief This function handles GPDMA1 Channel 1 global interrupt.
 */
void GPDMA1_Channel1_IRQHandler(void)
{
    /* USER CODE BEGIN GPDMA1_Channel1_IRQn 0 */

    /* USER CODE END GPDMA1_Channel1_IRQn 0 */
    HAL_DMA_IRQHandler(&handle_GPDMA1_Channel1);
    /* USER CODE BEGIN GPDMA1_Channel1_IRQn 1 */

    /* USER CODE END GPDMA1_Channel1_IRQn 1 */
}

/**
 * @brief This function handles GPDMA1 Channel 2 global interrupt.
 */
void GPDMA1_Channel2_IRQHandler(void)
{
    /* USER CODE BEGIN GPDMA1_Channel2_IRQn 0 */

    /* USER CODE END GPDMA1_Channel2_IRQn 0 */
    HAL_DMA_IRQHandler(&handle_GPDMA1_Channel2);
    /* USER CODE BEGIN GPDMA1_Channel2_IRQn 1 */

    /* USER CODE END GPDMA1_Channel2_IRQn 1 */
}

/**
 * @brief This function handles GPDMA1 Channel 3 global interrupt.
 */
void GPDMA1_Channel3_IRQHandler(void)
{
    /* USER CODE BEGIN GPDMA1_Channel3_IRQn 0 */

    /* USER CODE END GPDMA1_Channel3_IRQn 0 */
    HAL_DMA_IRQHandler(&handle_GPDMA1_Channel3);
    /* USER CODE BEGIN GPDMA1_Channel3_IRQn 1 */

    /* USER CODE END GPDMA1_Channel3_IRQn 1 */
}

/**
 * @brief This function handles USART1 global interrupt.
 */
void USART1_IRQHandler(void)
{
    /* USER CODE BEGIN USART1_IRQn 0 */

    /* USER CODE END USART1_IRQn 0 */
    HAL_UART_IRQHandler(&huart1);
    /* USER CODE BEGIN USART1_IRQn 1 */

    /* USER CODE END USART1_IRQn 1 */
}

/**
 * @brief This function handles USART2 global interrupt.
 */
void USART2_IRQHandler(void)
{
    /* USER CODE BEGIN USART2_IRQn 0 */

    /* USER CODE END USART2_IRQn 0 */
    HAL_UART_IRQHandler(&huart2);
    /* USER CODE BEGIN USART2_IRQn 1 */

    /* USER CODE END USART2_IRQn 1 */
}

/**
 * @brief This function handles TIM16 global interrupt.
 */
void TIM16_IRQHandler(void)
{
    /* USER CODE BEGIN TIM16_IRQn 0 */

    /* USER CODE END TIM16_IRQn 0 */
    /* Check whether update interrupt is pending */
    if (LL_TIM_IsActiveFlag_UPDATE(TIM16) == 1)
    {
        /* Clear the update interrupt flag */
        LL_TIM_ClearFlag_UPDATE(TIM16);

#if (CFG_SCM_SUPPORTED == 1)
        /* SCM HSE BEGIN */
        /* Update interrupt processing */
        SCM_HSE_SW_HSERDY_isr();
        /* SCM HSE END */
#endif /* CFG_SCM_SUPPORTED */
    }
    /* USER CODE BEGIN TIM16_IRQn 1 */

    /* USER CODE END TIM16_IRQn 1 */
}

/**
 * @brief This function handles PKA global interrupt.
 */
void PKA_IRQHandler(void)
{
    /* USER CODE BEGIN PKA_IRQn 0 */
    /* Check incoming interrupt */
    if (0u != LL_PKA_IsActiveFlag_PROCEND(PKA))
    {
        /* Clear the interrupt flag */
        LL_PKA_ClearFlag_PROCEND(PKA);

        /* Call the PKACTRL Callback */
        PKACTRL_EndOfProcessCb();
    }
    /* USER CODE END PKA_IRQn 0 */
    /* USER CODE BEGIN PKA_IRQn 1 */

    /* USER CODE END PKA_IRQn 1 */
}

/**
 * @brief This function handles ADC4 (12bits) global interrupt.
 */
void ADC4_IRQHandler(void)
{
    /* USER CODE BEGIN ADC4_IRQn 0 */
#if (CFG_JOYSTICK_SUPPORTED == 1)
    BSP_JOY_IRQHandler(JOY1, (JOYPin_TypeDef) 0);
#endif /* (CFG_JOYSTICK_SUPPORTED == 1) */
       /* USER CODE END ADC4_IRQn 0 */
       /* USER CODE BEGIN ADC4_IRQn 1 */

    /* USER CODE END ADC4_IRQn 1 */
}

/**
 * @brief This function handles 2.4GHz RADIO global interrupt.
 */
void RADIO_IRQHandler(void)
{
    /* USER CODE BEGIN RADIO_IRQn 0 */

    /* USER CODE END RADIO_IRQn 0 */

    if (NULL != radio_callback)
    {
        radio_callback();
    }

    LL_RCC_RADIO_DisableSleepTimerClock();
    __ISB();

    /* USER CODE BEGIN RADIO_IRQn 1 */

    /* USER CODE END RADIO_IRQn 1 */
}

/**
 * @brief This function handles PWR global WKUP pin interrupt.
 */
void WKUP_IRQHandler(void)
{
    /* USER CODE BEGIN WKUP_IRQn 0 */

    /* USER CODE END WKUP_IRQn 0 */
    HAL_PWR_WKUP_IRQHandler();
    /* USER CODE BEGIN WKUP_IRQn 1 */

    /* USER CODE END WKUP_IRQn 1 */
}

/**
 * @brief This function handles COMP1 and COMP2 through EXTI Lines interrupts.
 */
void COMP_IRQHandler(void)
{
    /* USER CODE BEGIN COMP_IRQn 0 */

    /* USER CODE END COMP_IRQn 0 */

    /* Disable SW radio low interrupt to prevent nested calls */
    NVIC_DisableIRQ(RADIO_SW_LOW_INTR_NUM);

    if (NULL != low_isr_callback)
    {
        low_isr_callback();
    }

    /* Check if nested SW radio low interrupt has been requested*/
    if (radio_sw_low_isr_is_running_high_prio != 0)
    {
        HAL_NVIC_SetPriority((IRQn_Type) RADIO_SW_LOW_INTR_NUM, RADIO_INTR_PRIO_LOW, 0);
        radio_sw_low_isr_is_running_high_prio = 0;
    }

    /* Re-enable SW radio low interrupt */
    NVIC_EnableIRQ(RADIO_SW_LOW_INTR_NUM);

    /* USER CODE BEGIN COMP_IRQn 1 */

    /* USER CODE END COMP_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
