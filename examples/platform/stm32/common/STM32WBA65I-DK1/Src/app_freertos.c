/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : FreeRTOS applicative file
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
#include "app_freertos.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_ble.h"
#include "ll_sys_if.h"
#include "timer_if.h"
#include "app_bsp.h"
#include "log_module.h"

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
const osThreadAttr_t BSPInitTask_attributes = {
  .name = "BSPInitTask",
  .priority = (osPriority_t) osPriorityHigh,
  .stack_size = 256 * 4
};

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
  BSPInitTaskHandle = osThreadNew(BSPInitTask_Entry, NULL, &BSPInitTask_attributes);
  if ( BSPInitTaskHandle == NULL )
  {
    LOG_DEBUG_APP( "ERROR FREERTOS : BSP_Init START THREAD CREATION FAILED" );
    while(1);
  }
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
}

/**
 * @brief   System Tasks Initialisations
 */
void BSPInitTask_Entry( void * argument )
{
  APP_BSP_Init();

  osThreadExit();
}
/* USER CODE END Application */

