/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    peripheral_init.c
  * @author  MCD Application Team
  * @brief   tbd module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "app_conf.h"
#include "peripheral_init.h"
#include "main.h"
#include "crc_ctrl.h"
#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
#include "adc_ctrl.h"
#endif /* USE_TEMPERATURE_BASED_RADIO_CALIBRATION */
/* Private includes -----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32wba65i_discovery.h"
/* USER CODE END Includes */

/* External variables --------------------------------------------------------*/
#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
extern ADC_HandleTypeDef hadc4;
#endif /* USE_TEMPERATURE_BASED_RADIO_CALIBRATION */
extern CRC_HandleTypeDef hcrc;
extern RAMCFG_HandleTypeDef hramcfg_SRAM1;
extern RNG_HandleTypeDef hrng;

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Functions Definition ------------------------------------------------------*/

/**
  * @brief  Configure the SoC peripherals at Standby mode exit.
  * @param  None
  * @retval None
  */
void MX_StandbyExit_PeripheralInit(void)
{
  /* USER CODE BEGIN MX_STANDBY_EXIT_PERIPHERAL_INIT_1 */

  /* USER CODE END MX_STANDBY_EXIT_PERIPHERAL_INIT_1 */

#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
  memset(&hadc4, 0, sizeof(hadc4));
#endif /* USE_TEMPERATURE_BASED_RADIO_CALIBRATION */
  memset(&hcrc, 0, sizeof(hcrc));
  memset(&hramcfg_SRAM1, 0, sizeof(hramcfg_SRAM1));
  memset(&hrng, 0, sizeof(hrng));

  MX_GPIO_Init();
  MX_RAMCFG_Init();
#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
  MX_ADC4_Init();
#endif /* USE_TEMPERATURE_BASED_RADIO_CALIBRATION */
  MX_RNG_Init();
  MX_CRC_Init();
  MX_ICACHE_Init();
  CRCCTRL_Init();
#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
  ADCCTRL_Init();
#endif /* USE_TEMPERATURE_BASED_RADIO_CALIBRATION */

#if (CFG_DEBUGGER_LEVEL == 0)
  GPIO_InitTypeDef DbgIOsInit = {0};
  DbgIOsInit.Mode = GPIO_MODE_ANALOG;
  DbgIOsInit.Pull = GPIO_NOPULL;
  DbgIOsInit.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  __HAL_RCC_GPIOA_CLK_ENABLE();
  HAL_GPIO_Init(GPIOA, &DbgIOsInit);

  DbgIOsInit.Mode = GPIO_MODE_ANALOG;
  DbgIOsInit.Pull = GPIO_NOPULL;
  DbgIOsInit.Pin = GPIO_PIN_3|GPIO_PIN_4;
  __HAL_RCC_GPIOB_CLK_ENABLE();
  HAL_GPIO_Init(GPIOB, &DbgIOsInit);
#endif /* CFG_DEBUGGER_LEVEL */
  /* USER CODE BEGIN MX_STANDBY_EXIT_PERIPHERAL_INIT_2 */
#if (CFG_BUTTON_SUPPORTED == 1)
  BSP_PB_Init(B1, BUTTON_MODE_EXTI);
  BSP_PB_Init(B2, BUTTON_MODE_EXTI);
  BSP_PB_Init(B3, BUTTON_MODE_EXTI);
#endif
  /* USER CODE END MX_STANDBY_EXIT_PERIPHERAL_INIT_2 */
}
