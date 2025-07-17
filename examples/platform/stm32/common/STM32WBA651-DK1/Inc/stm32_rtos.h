/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32_rtos.h
  * @author  MCD Application Team
  * @brief   Include file for all RTOS/Sequencer can be used on WBA
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32_RTOS_H
#define STM32_RTOS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "app_freertos.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macros -----------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define CFG_TASK_PRIO_LINK_LAYER                osPriorityRealtime
#define CFG_TASK_PRIO_US_ALARM                  osPriorityRealtime
#define CFG_TASK_PRIO_HCI_ASYNCH_EVT            osPriorityHigh
#define CFG_TASK_PRIO_BLE_STACK                 osPriorityHigh
#define CFG_TASK_PRIO_BLE_TIMER                 osPriorityHigh
#define CFG_TASK_PRIO_ALARM                     osPriorityHigh
#define CFG_TASK_PRIO_TASKLETS                  osPriorityHigh
#define CFG_TASK_PRIO_HW_RNG                    osPriorityAboveNormal
#define CFG_TASK_PRIO_PROCESSPKA                osPriorityAboveNormal
#define CFG_TASK_PRIO_BPKA                      osPriorityAboveNormal
#define CFG_TASK_PRIO_AMM_BCKG                  osPriorityNormal
#define CFG_TASK_PRIO_FLASH_MANAGER_BCKGND      osPriorityNormal
#define CFG_TASK_PRIO_CLI_UART                  osPriorityNormal
#define LINK_LAYER_TEMP_MEAS_TASK_PRIO          osPriorityNormal

#define TASK_PRIO_BUTTON_Bx                     osPriorityNormal
#define TASK_PREEMP_BUTTON_Bx                   osPriorityNormal
#define TASK_PRIO_JOYSTICK_x					osPriorityNormal
#define TASK_PREEMP_JOYSTICK_x                  osPriorityNormal

/* USER CODE BEGIN TASK_Priority_Define */


#define RTOS_STACK_SIZE_X_LARGE                 ( 1024u * 3u + 512u)
#define RTOS_STACK_SIZE_LARGE                   ( 1024u * 3u )
#define RTOS_STACK_SIZE_ENHANCED                ( 1024u * 2u )
#define RTOS_STACK_SIZE_NORMAL                  ( 1024u )
#define RTOS_STACK_SIZE_REDUCED                 ( 512u )
#define RTOS_STACK_SIZE_SMALL                   ( 384u )

#define RTOS_MAX_THREAD                         20
  
/* Tasks stack sizes by default  */
#define TASK_LINK_LAYER_STACK_SIZE              RTOS_STACK_SIZE_LARGE //RTOS_STACK_SIZE_ENHANCED
#define TASK_HW_RNG_STACK_SIZE                  RTOS_STACK_SIZE_REDUCED
#define TASK_PROCESSPKA_STACK_SIZE              RTOS_STACK_SIZE_ENHANCED        // to adjust
#define TASK_ALARM_STACK_SIZE                   RTOS_STACK_SIZE_LARGE //RTOS_STACK_SIZE_ENHANCED
#define TASK_ALARM_US_STACK_SIZE                RTOS_STACK_SIZE_NORMAL
#define TASK_TASKLETS_STACK_SIZE                RTOS_STACK_SIZE_X_LARGE
#define TASK_CLI_UART_STACK_SIZE                RTOS_STACK_SIZE_NORMAL
#define TASK_SEND_COAP_MSG_STACK_SIZE           RTOS_STACK_SIZE_NORMAL
#define TASK_BPKA_STACK_SIZE                    RTOS_STACK_SIZE_NORMAL
#define TASK_HCI_ASYNCH_EVT_STACK_SIZE          RTOS_STACK_SIZE_ENHANCED
#define TASK_BLE_STACK_STACK_SIZE               RTOS_STACK_SIZE_ENHANCED
#define TASK_BLE_TIMER_STACK_SIZE               RTOS_STACK_SIZE_NORMAL
#define TASK_FLASH_MANAGER_BCKGND_STACK_SIZE    RTOS_STACK_SIZE_LARGE//RTOS_STACK_SIZE_REDUCED
#define TASK_AMM_BCKG_STACK_SIZE                RTOS_STACK_SIZE_REDUCED
#define TASK_ADVERTISING_STACK_SIZE             RTOS_STACK_SIZE_NORMAL
#define TASK_LINK_LAYER_TEMP_MEAS_STACK_SIZE    RTOS_STACK_SIZE_NORMAL
#define TASK_STACK_SIZE_BUTTON_Bx				RTOS_STACK_SIZE_NORMAL
#define TASK_STACK_SIZE_JOYSTICK_x				RTOS_STACK_SIZE_NORMAL


/* USER CODE BEGIN TASK_Size_Define */
#define TASK_DEFAULT_ATTR_BITS                  ( 0u )
#define TASK_DEFAULT_CB_MEM                     ( 0u )
#define TASK_DEFAULT_CB_SIZE                    ( 0u )
#define TASK_DEFAULT_STACK_MEM                  ( 0u )
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
