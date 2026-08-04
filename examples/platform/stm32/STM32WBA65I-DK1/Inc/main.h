/* USER CODE BEGIN Header */
/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License,
 * Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy
 * of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to
 * in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *
 * limitations under the License.
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "app_common.h"
#include "app_conf.h"
#include "app_debug.h"
#include "app_entry.h"
#include "stm32wbaxx_hal.h"

#include "stm32wbaxx_ll_bus.h"
#include "stm32wbaxx_ll_cortex.h"
#include "stm32wbaxx_ll_dma.h"
#include "stm32wbaxx_ll_gpio.h"
#include "stm32wbaxx_ll_icache.h"
#include "stm32wbaxx_ll_pwr.h"
#include "stm32wbaxx_ll_rcc.h"
#include "stm32wbaxx_ll_system.h"
#include "stm32wbaxx_ll_tim.h"
#include "stm32wbaxx_ll_utils.h"

#include "stm32wbaxx_ll_exti.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_bsp.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);
void MX_ICACHE_Init(void);
void MX_RAMCFG_Init(void);
void MX_RNG_Init(void);
void MX_RTC_Init(void);
void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);
void MX_CRC_Init(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define User_Button_Pin GPIO_PIN_3
#define User_Button_GPIO_Port GPIOA
#define SPI3_SCK_Pin GPIO_PIN_0
#define SPI3_SCK_GPIO_Port GPIOA
#define LD5_Pin GPIO_PIN_9
#define LD5_GPIO_Port GPIOD
#define LD6_Pin GPIO_PIN_8
#define LD6_GPIO_Port GPIOD
#define SPI3_MOSI_Pin GPIO_PIN_8
#define SPI3_MOSI_GPIO_Port GPIOB
#define OSC32_OUT_Pin GPIO_PIN_15
#define OSC32_OUT_GPIO_Port GPIOC
#define OSC32_IN_Pin GPIO_PIN_14
#define OSC32_IN_GPIO_Port GPIOC
#define RST_DISP_Pin GPIO_PIN_3
#define RST_DISP_GPIO_Port GPIOE
#define CS_DISP_Pin GPIO_PIN_1
#define CS_DISP_GPIO_Port GPIOE
#define D_C_DISP_Pin GPIO_PIN_0
#define D_C_DISP_GPIO_Port GPIOE
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define I2C1_SCL_Pin GPIO_PIN_2
#define I2C1_SCL_GPIO_Port GPIOB
#define I2C1_SDA_Pin GPIO_PIN_1
#define I2C1_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
