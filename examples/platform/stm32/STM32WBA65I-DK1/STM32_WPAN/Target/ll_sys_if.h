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
#ifndef LL_SYS_IF_H
#define LL_SYS_IF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "cmsis_os2.h"
#include "stdint.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
#define DRIFT_TIME_DEFAULT (14)
#define DRIFT_TIME_EXTRA_LSI2 (9)
#define DRIFT_TIME_EXTRA_GCC_DEBUG (6)

#define EXEC_TIME_DEFAULT (28)
#define EXEC_TIME_EXTRA_LSI2 (3)
#define EXEC_TIME_EXTRA_GCC_DEBUG (4)

#define SCHDL_TIME_DEFAULT (20)
/* USER CODE BEGIN EC */
#define DRIFT_TIME_OPTIMIZED (13)
#define EXEC_TIME_OPTIMIZED (20)

#if DRIFT_TIME_EXTRA_GCC_DEBUG
#undef DRIFT_TIME_EXTRA_GCC_DEBUG
#define DRIFT_TIME_EXTRA_GCC_DEBUG (5)
#endif

#if EXEC_TIME_EXTRA_GCC_DEBUG
#undef EXEC_TIME_EXTRA_GCC_DEBUG
#define EXEC_TIME_EXTRA_GCC_DEBUG (1)
#endif
/* USER CODE END EC */

/* External variables --------------------------------------------------------*/
extern osMutexId_t LinkLayerMutex;

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
void ll_sys_bg_temperature_measurement(void);
#endif /* USE_TEMPERATURE_BASED_RADIO_CALIBRATION */
#if defined(STM32WBA52xx) || defined(STM32WBA54xx) || defined(STM32WBA55xx) || defined(STM32WBA65xx)
/**
 * @brief Apply CTE degradation settings
 * @param  None
 * @retval None
 */
void ll_sys_apply_cte_settings(void);
#endif /* defined(STM32WBA52xx) || defined(STM32WBA54xx) || defined(STM32WBA55xx) || defined(STM32WBA65xx) */
#if (CFG_LPM_STANDBY_SUPPORTED == 0)
void ll_sys_get_ble_profile_statistics(uint32_t * exec_time, uint32_t * drift_time, uint32_t * average_drift_time, uint8_t reset);
#endif

void ll_sys_set_rtl_polling_time(uint8_t rtl_polling_time);
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /*LL_SYS_IF_H */
