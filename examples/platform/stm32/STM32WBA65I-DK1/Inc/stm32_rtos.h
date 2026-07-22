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
#ifndef STM32_RTOS_H
#define STM32_RTOS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "app_freertos.h"
#include "cmsis_os2.h"
#include "task.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* FreeRTOS priorities by default  */
#define TASK_PRIO_WPAN osPriorityNormal

/* USER CODE BEGIN TASK_Priority_Define */
#define TASK_PRIO_JOYSTICK_x osPriorityNormal3

/* USER CODE END TASK_Priority_Define */

#define RTOS_MAX_THREAD (20u)

#define RTOS_STACK_SIZE_LARGE (1024u * 3u)
#define RTOS_STACK_SIZE_MODERATE (2048u)
#define RTOS_STACK_SIZE_NORMAL (1024u)
#define RTOS_STACK_SIZE_REDUCED (512u)
#define RTOS_STACK_SIZE_SMALL (256u)
#define RTOS_STACK_SIZE_TINY (configMINIMAL_STACK_SIZE)

/* Tasks stack sizes by default  */
#define TASK_STACK_SIZE_WPAN RTOS_STACK_SIZE_LARGE
/* USER CODE BEGIN TASK_Size_Define */
#define TASK_STACK_SIZE_JOYSTICK_x RTOS_STACK_SIZE_NORMAL

/* USER CODE END TASK_Size_Define */

/* Attributes needed by CMSIS */
#define TASK_DEFAULT_ATTR_BITS (0u)
#define TASK_DEFAULT_CB_MEM (0u)
#define TASK_DEFAULT_CB_SIZE (0u)
#define TASK_DEFAULT_STACK_MEM (0u)

#define SEMAPHORE_DEFAULT_ATTR_BITS (0u)
#define SEMAPHORE_DEFAULT_CB_MEM (0u)
#define SEMAPHORE_DEFAULT_CB_SIZE (0u)

#define MUTEX_DEFAULT_ATTR_BITS (0u)
#define MUTEX_DEFAULT_CB_MEM (0u)
#define MUTEX_DEFAULT_CB_SIZE (0u)

/* USER CODE BEGIN Attributes_Define */

/* USER CODE END Attributes_Define */

/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macros -----------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Exported functions prototypes ---------------------------------------------*/
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // STM32_RTOS_H
