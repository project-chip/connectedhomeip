/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    peripheral_init.c
 * @author  MCD Application Team
 * @brief   tbd module
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
#include "peripheral_init.h"
#include "app_conf.h"
#include "crc_ctrl.h"
#include "main.h"
#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
#include "adc_ctrl.h"
#endif /* USE_TEMPERATURE_BASED_RADIO_CALIBRATION */
#if (CFG_LPM_WAKEUP_TIME_PROFILING == 1)
#include "stm32_lpm_if.h"
#endif /* CFG_LPM_WAKEUP_TIME_PROFILING */
/* Private includes -----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* External variables --------------------------------------------------------*/
extern RAMCFG_HandleTypeDef hramcfg_SRAM1;
extern RNG_HandleTypeDef hrng;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Functions Definition ------------------------------------------------------*/

#if (CFG_LPM_STANDBY_SUPPORTED == 1)
/**
 * @brief  Configure the SoC peripherals at Standby mode exit.
 * @param  None
 * @retval None
 */
void MX_StandbyExit_PeripheralInit(void)
{
    /* USER CODE BEGIN MX_STANDBY_EXIT_PERIPHERAL_INIT_1 */

    /* USER CODE END MX_STANDBY_EXIT_PERIPHERAL_INIT_1 */

#if (CFG_LPM_WAKEUP_TIME_PROFILING == 1)
#if (CFG_LPM_STANDBY_SUPPORTED == 1)
    /* Do not configure sysTick if currently used by wakeup time profiling mechanism */
    if (LPM_is_wakeup_time_profiling_done() != 0)
    {
        /* Select SysTick source clock */
        HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_LSE);

        /* Initialize SysTick */
        if (HAL_InitTick(TICK_INT_PRIORITY) != HAL_OK)
        {
            assert_param(0);
        }
    }
#endif /* CFG_LPM_STANDBY_SUPPORTED */
#else
    /* Select SysTick source clock */
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_LSE);

    /* Initialize SysTick */
    if (HAL_InitTick(TICK_INT_PRIORITY) != HAL_OK)
    {
        assert_param(0);
    }
#endif /* CFG_LPM_WAKEUP_TIME_PROFILING */

#if (CFG_DEBUGGER_LEVEL == 0)
    /* Setup GPIOA 13, 14, 15 in Analog no pull */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIOA->PUPDR &= ~0xFC000000;
    GPIOA->MODER |= 0xFC000000;
    __HAL_RCC_GPIOA_CLK_DISABLE();

    /* Setup GPIOB 3, 4 in Analog no pull */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIOB->PUPDR &= ~0x3C0;
    GPIOB->MODER |= 0x3C0;
    __HAL_RCC_GPIOB_CLK_DISABLE();
#endif /* CFG_DEBUGGER_LEVEL */

    memset(&hramcfg_SRAM1, 0, sizeof(hramcfg_SRAM1));
#if (CFG_LOG_SUPPORTED == 1)
    memset(&huart1, 0, sizeof(huart1));
#endif
    memset(&huart2, 0, sizeof(huart2));

    MX_ICACHE_Init();
    MX_RAMCFG_Init();
#if (CFG_LOG_SUPPORTED == 1)
    MX_USART1_UART_Init();
#endif
    MX_USART2_UART_Init();
    CRCCTRL_Init();
#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
    ADCCTRL_Init();
#endif /* USE_TEMPERATURE_BASED_RADIO_CALIBRATION */

    /* USER CODE BEGIN MX_STANDBY_EXIT_PERIPHERAL_INIT_2 */

    /* USER CODE END MX_STANDBY_EXIT_PERIPHERAL_INIT_2 */
}
#endif /* (CFG_LPM_STANDBY_SUPPORTED == 1) */

#if (CFG_LPM_STOP2_SUPPORTED == 1)
void MX_Stop2Exit_PeripheralInit(void)
{
    /* USER CODE BEGIN MX_STOP2_EXIT_PERIPHERAL_INIT_1 */
    /* USER CODE END MX_STOP2_EXIT_PERIPHERAL_INIT_1 */

    memset(&huart2, 0, sizeof(huart2));

#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
    ADCCTRL_Init();
#endif /* USE_TEMPERATURE_BASED_RADIO_CALIBRATION */

    /* USER CODE BEGIN MX_STOP2_EXIT_PERIPHERAL_INIT_2 */
    /* USER CODE END MX_STOP2_EXIT_PERIPHERAL_INIT_2 */
}
#endif /* (CFG_LPM_STOP2_SUPPORTED == 1) */
