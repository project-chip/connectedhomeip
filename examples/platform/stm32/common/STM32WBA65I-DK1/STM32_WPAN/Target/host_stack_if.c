/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    host_stack_if.c
  * @author  MCD Application Team
  * @brief : Source file for the stack tasks
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
#include "host_stack_if.h"
#include "app_conf.h"
#include "ll_sys.h"
#include "app_entry.h"
#include "app_ble.h"
#include "auto/ble_raw_api.h"
#include "cmsis_os2.h"

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
