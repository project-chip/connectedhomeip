/* USER CODE BEGIN Header */
/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "host_stack_if.h"
#include "app_ble.h"
#include "app_conf.h"
#include "app_entry.h"
#include "auto/ble_raw_api.h"
#include "cmsis_os2.h"
#include "ll_sys.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* External variables --------------------------------------------------------*/
/**
 * @brief  Missed HCI event flag
 */
extern uint8_t missed_hci_event_flag;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* External function prototypes -----------------------------------------------*/

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/**
 * @brief  Host stack processing request from Link Layer.
 * @param  None
 * @retval None
 */
void HostStack_Process(void)
{
    /* USER CODE BEGIN HostStack_Process 0 */

    /* USER CODE END HostStack_Process 0 */

    /* Process BLE Host stack */
    BleStackCB_Process();

    /* USER CODE BEGIN HostStack_Process 1 */

    /* USER CODE END HostStack_Process 1 */
}

/**
 * @brief  BLE Host stack processing callback.
 * @param  None
 * @retval None
 */
void BleStackCB_Process(void)
{
    /* USER CODE BEGIN BleStackCB_Process 0 */

    /* USER CODE END BleStackCB_Process 0 */
    if (missed_hci_event_flag)
    {
        missed_hci_event_flag = 0;
        /* missed event, Link Layer fifo full */
    }
    /* BLE Host stack processing through background task */
    osThreadFlagsSet(WpanTaskHandle, 1U << CFG_RTOS_FLAG_BLEhost);

    /* USER CODE BEGIN BleStackCB_Process 1 */

    /* USER CODE END BleStackCB_Process 1 */
}
