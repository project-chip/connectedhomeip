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

/* Includes ------------------------------------------------------------------*/
#include "app_freertos.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_ble.h"
#include "app_bsp.h"
#include "ll_sys_if.h"
#include "log_module.h"
#include "timer_if.h"

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
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

osThreadId_t BSPInitTaskHandle;
const osThreadAttr_t BSPInitTask_attributes = { .name       = "BSPInitTask",
                                                .priority   = (osPriority_t) osPriorityHigh,
                                                .stack_size = 256 * 4 };

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void)
{
    /* USER CODE BEGIN Init */
    BSPInitTaskHandle = osThreadNew(BSPInitTask_Entry, NULL, &BSPInitTask_attributes);
    if (BSPInitTaskHandle == NULL)
    {
        LOG_DEBUG_APP("ERROR FREERTOS : BSP_Init START THREAD CREATION FAILED");
        while (1)
            ;
    }
    /* USER CODE END Init */

    /* USER CODE BEGIN RTOS_MUTEX */
}

/**
 * @brief   System Tasks Initialisations
 */
void BSPInitTask_Entry(void * argument)
{
    APP_BSP_Init();

    osThreadExit();
}
/* USER CODE END Application */
