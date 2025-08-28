/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.cpp
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
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
#include "app_common.h"
#include "main.h"
#include "stm32_rtos.h"
#include "STM32FreeRtosHooks.h"
#include "app_entry.h"
#include "AppTask.h"
#include "cmsis_os2.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32wba65i_discovery_bus.h"

#if (OTA_EXTERNAL_FLASH_ENABLE == 1)
#include "Driver_Flash.h"
#include "stm32wba65i_discovery_conf.h"
#endif  /* (OTA_EXTERNAL_FLASH_ENABLE == 1) */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CRC_HandleTypeDef hcrc;
RAMCFG_HandleTypeDef hramcfg_SRAM1;
RNG_HandleTypeDef hrng;
RTC_HandleTypeDef hrtc;
UART_HandleTypeDef huart1;
DMA_HandleTypeDef handle_GPDMA1_Channel1;
DMA_HandleTypeDef handle_GPDMA1_Channel0;
CRYP_HandleTypeDef hcryp;
PKA_HandleTypeDef hpka;
__ALIGN_BEGIN static const uint32_t pKeyAES[4] __ALIGN_END = {
                            0x00000000,0x00000000,0x00000000,0x00000000};
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_AES_Init(void);
static void MX_PKA_Init(void);
#if (OTA_EXTERNAL_FLASH_ENABLE == 1)
static void SPI_Init(void);
#endif /* (OTA_EXTERNAL_FLASH_ENABLE == 1) */

//static void MX_ADC4_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();
	/* Config code for STM32_WPAN (HSE Tuning must be done before system clock configuration) */
	MX_APPE_Config();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* Configure the peripherals common clocks */
	PeriphCommonClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_GPDMA1_Init();
	MX_RAMCFG_Init();
	MX_RTC_Init();
	MX_RNG_Init();
	MX_ICACHE_Init();
	MX_CRC_Init();
	MX_USART1_UART_Init();
	MX_PKA_Init();
	MX_AES_Init();

	/* USER CODE BEGIN 2 */
#if (OTA_EXTERNAL_FLASH_ENABLE == 1)
  /* Configure SPI module */
  SPI_Init();
#endif /* (OTA_EXTERNAL_FLASH_ENABLE == 1) */	
	/* USER CODE END 2 */
	/* Init scheduler */
	osKernelInitialize();

	freertos_mbedtls_init();


	/* Init code for STM32_WPAN */
	MX_APPE_Init(NULL);

	/* Start scheduler */
	osKernelStart();


	/* We should never get here as control is now taken by the scheduler */
	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	HAL_PWREx_ConfigSupply(PWR_SMPS_SUPPLY);

	/** Configure the main internal regulator output voltage
	 */
	if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1)
			!= HAL_OK) {
		Error_Handler();
	}

	/** Configure LSE Drive Capability
	 */
	HAL_PWR_EnableBkUpAccess();
	__HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_MEDIUMLOW);

	/** Initializes the CPU, AHB and APB busses clocks
	 */

	/* Init PLL here instead of using the scm config  so that PLL is up faster
	 * config will be saved anyway by scm_init
	 */

	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI
			| RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEDiv = RCC_HSE_DIV1;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	/* 100MHz PLL config */
	RCC_OscInitStruct.PLL1.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL1.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL1.PLLM = 4;
	RCC_OscInitStruct.PLL1.PLLN = 25;
	RCC_OscInitStruct.PLL1.PLLP = 2;
	RCC_OscInitStruct.PLL1.PLLQ = 2;
	RCC_OscInitStruct.PLL1.PLLR = 2;
	RCC_OscInitStruct.PLL1.PLLFractional = 0;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_PCLK7
			| RCC_CLOCKTYPE_HCLK5;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB7CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.AHB5_PLL1_CLKDivider = RCC_SYSCLK_PLL1_DIV4;/*should not be >32MHz*/
	RCC_ClkInitStruct.AHB5_HSEHSI_CLKDivider = RCC_SYSCLK_HSEHSI_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief Peripherals Common Clock Configuration
 * @retval None
 */
void PeriphCommonClock_Config(void) {
	RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

	/** Initializes the peripherals clock
	 */
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RADIOST;
	PeriphClkInit.RadioSlpTimClockSelection = RCC_RADIOSTCLKSOURCE_LSE;

	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
		Error_Handler();
	}
}


/**
 * @brief CRC Initialization Function
 * @param None
 * @retval None
 */
void MX_CRC_Init(void) {

	/* USER CODE BEGIN CRC_Init 0 */

	/* USER CODE END CRC_Init 0 */

	/* USER CODE BEGIN CRC_Init 1 */

	/* USER CODE END CRC_Init 1 */
	hcrc.Instance = CRC;
	hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_DISABLE;
	hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
	hcrc.Init.GeneratingPolynomial = 7607;
	hcrc.Init.CRCLength = CRC_POLYLENGTH_16B;
	hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
	hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
	hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_WORDS;
	if (HAL_CRC_Init(&hcrc) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN CRC_Init 2 */

	/* USER CODE END CRC_Init 2 */

}

/**
 * @brief GPDMA1 Initialization Function
 * @param None
 * @retval None
 */
void MX_GPDMA1_Init(void) {

	/* USER CODE BEGIN GPDMA1_Init 0 */

	/* USER CODE END GPDMA1_Init 0 */

	/* Peripheral clock enable */
	__HAL_RCC_GPDMA1_CLK_ENABLE();

	/* GPDMA1 interrupt Init */
	HAL_NVIC_SetPriority(GPDMA1_Channel0_IRQn, 6, 0);
	HAL_NVIC_EnableIRQ(GPDMA1_Channel0_IRQn);
	HAL_NVIC_SetPriority(GPDMA1_Channel1_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(GPDMA1_Channel1_IRQn);

	/* USER CODE BEGIN GPDMA1_Init 1 */

	/* USER CODE END GPDMA1_Init 1 */
	/* USER CODE BEGIN GPDMA1_Init 2 */

	/* USER CODE END GPDMA1_Init 2 */

}

/**
 * @brief ICACHE Initialization Function
 * @param None
 * @retval None
 */
void MX_ICACHE_Init(void) {

	/* USER CODE BEGIN ICACHE_Init 0 */

	/* USER CODE END ICACHE_Init 0 */

	/* USER CODE BEGIN ICACHE_Init 1 */
	/* No retention for ICACHE in stop mode */
	LL_PWR_SetICacheRAMStopRetention(LL_PWR_ICACHERAM_STOP_NO_RETENTION);
	/* USER CODE END ICACHE_Init 1 */

	/** Enable instruction cache in 1-way (direct mapped cache)
	 */
	if (HAL_ICACHE_ConfigAssociativityMode(ICACHE_1WAY) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_ICACHE_Enable() != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN ICACHE_Init 2 */

	/* USER CODE END ICACHE_Init 2 */

}

/**
 * @brief RAMCFG Initialization Function
 * @param None
 * @retval None
 */
void MX_RAMCFG_Init(void) {

	/* USER CODE BEGIN RAMCFG_Init 0 */

	/* USER CODE END RAMCFG_Init 0 */

	/* USER CODE BEGIN RAMCFG_Init 1 */

	/* USER CODE END RAMCFG_Init 1 */

	/** Initialize RAMCFG SRAM1
	 */
	hramcfg_SRAM1.Instance = RAMCFG_SRAM1;
	if (HAL_RAMCFG_Init(&hramcfg_SRAM1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN RAMCFG_Init 2 */

	/* USER CODE END RAMCFG_Init 2 */

}

/**
 * @brief RNG Initialization Function
 * @param None
 * @retval None
 */
void MX_RNG_Init(void) {

	/* USER CODE BEGIN RNG_Init 0 */
	memset(&hrng, 0, sizeof(hrng));
	/* USER CODE END RNG_Init 0 */

	/* USER CODE BEGIN RNG_Init 1 */

	/* USER CODE END RNG_Init 1 */
	hrng.Instance = RNG;
	hrng.Init.ClockErrorDetection = RNG_CED_DISABLE;
	if (HAL_RNG_Init(&hrng) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN RNG_Init 2 */

	/* Disable RNG peripheral and its RCC clock */
	HW_RNG_Disable();

	/* USER CODE END RNG_Init 2 */

}

/**
  * @brief PKA Initialization Function
  * @param None
  * @retval None
  */
static void MX_PKA_Init(void)
{

  /* USER CODE BEGIN PKA_Init 0 */

  /* USER CODE END PKA_Init 0 */

  /* USER CODE BEGIN PKA_Init 1 */

  /* USER CODE END PKA_Init 1 */
  hpka.Instance = PKA;
  if (HAL_PKA_Init(&hpka) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN PKA_Init 2 */

  /* USER CODE END PKA_Init 2 */

}

/**
 * @brief RTC Initialization Function
 * @param None
 * @retval None
 */
void MX_RTC_Init(void) {

	/* USER CODE BEGIN RTC_Init 0 */

	/* USER CODE END RTC_Init 0 */

	RTC_PrivilegeStateTypeDef privilegeState = { 0 };
	RTC_AlarmTypeDef sAlarm = { 0 };

	/* USER CODE BEGIN RTC_Init 1 */

	/* USER CODE END RTC_Init 1 */

	/** Initialize RTC Only
	 */
	hrtc.Instance = RTC;
	hrtc.Init.AsynchPrediv = 31;
	hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
	hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
	hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
	hrtc.Init.BinMode = RTC_BINARY_ONLY;
	if (HAL_RTC_Init(&hrtc) != HAL_OK) {
		Error_Handler();
	}
	privilegeState.rtcPrivilegeFull = RTC_PRIVILEGE_FULL_NO;
	privilegeState.backupRegisterPrivZone = RTC_PRIVILEGE_BKUP_ZONE_NONE;
	privilegeState.backupRegisterStartZone2 = RTC_BKP_DR0;
	privilegeState.backupRegisterStartZone3 = RTC_BKP_DR0;
	if (HAL_RTCEx_PrivilegeModeSet(&hrtc, &privilegeState) != HAL_OK) {
		Error_Handler();
	}

	/* USER CODE BEGIN Check_RTC_BKUP */

	/* USER CODE END Check_RTC_BKUP */

	/** Initialize RTC and set the Time and Date
	 */
	if (HAL_RTCEx_SetSSRU_IT(&hrtc) != HAL_OK) {
		Error_Handler();
	}

	/** Enable the Alarm A
	 */
	sAlarm.BinaryAutoClr = RTC_ALARMSUBSECONDBIN_AUTOCLR_NO;
	sAlarm.AlarmTime.SubSeconds = 0x0;
	sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
	sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDBINMASK_NONE;
	sAlarm.Alarm = RTC_ALARM_A;
	if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN RTC_Init 2 */

	/* USER CODE END RTC_Init 2 */

}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
void MX_USART1_UART_Init(void) {

	/* USER CODE BEGIN USART1_Init 0 */

	/* USER CODE END USART1_Init 0 */

	/* USER CODE BEGIN USART1_Init 1 */

	/* USER CODE END USART1_Init 1 */
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_8;
	huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
	huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart1) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8)
			!= HAL_OK) {
		Error_Handler();
	}
	if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8)
			!= HAL_OK) {
		Error_Handler();
	}
	if (HAL_UARTEx_EnableFifoMode(&huart1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART1_Init 2 */

	/* USER CODE END USART1_Init 2 */

}

/**
  * @brief AES Initialization Function
  * @param None
  * @retval None
  */
static void MX_AES_Init(void)
{

  /* USER CODE BEGIN AES_Init 0 */

  /* USER CODE END AES_Init 0 */

  /* USER CODE BEGIN AES_Init 1 */

  /* USER CODE END AES_Init 1 */
  hcryp.Instance = AES;
  hcryp.Init.DataType = CRYP_NO_SWAP;
  hcryp.Init.KeySize = CRYP_KEYSIZE_128B;
  hcryp.Init.pKey = (uint32_t *)pKeyAES;
  hcryp.Init.Algorithm = CRYP_AES_ECB;
  hcryp.Init.DataWidthUnit = CRYP_DATAWIDTHUNIT_WORD;
  hcryp.Init.HeaderWidthUnit = CRYP_HEADERWIDTHUNIT_WORD;
  hcryp.Init.KeyIVConfigSkip = CRYP_KEYIVCONFIG_ALWAYS;
  hcryp.Init.KeyMode = CRYP_KEYMODE_NORMAL;
  if (HAL_CRYP_Init(&hcryp) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN AES_Init 2 */

  /* USER CODE END AES_Init 2 */

}
/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
void MX_GPIO_Init(void) {
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	/*RT DEBUG GPIO_Init */
	RT_DEBUG_GPIO_Init();

	/* USER CODE BEGIN MX_GPIO_Init_2 */

	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  Error_Handler();
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

#if (OTA_EXTERNAL_FLASH_ENABLE == 1)
/**
  * @brief SPI Initialization Function
  * @param None
  * @retval None
  */
static void SPI_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  M95P32_EEPROM_SPI_CS_RCC();

  /*Configure GPIO pin : M95P32_EEPROM_SPI_CS_PIN */
  GPIO_InitStruct.Pin = M95P32_EEPROM_SPI_CS_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(M95P32_EEPROM_SPI_CS_PORT, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}
#endif /* (OTA_EXTERNAL_FLASH_ENABLE == 1) */
