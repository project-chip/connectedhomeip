/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    host_stack_if.h
  * @author  MCD Application Team
  * @brief : This file contains the interface for the stack tasks
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

#ifndef HOST_STACK_IF_H
#define HOST_STACK_IF_H

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ll_sys_if.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* Halt if any aci/hci functions call is made under ISR context, for debug purpose. */
/* Acquire Link Layer Mutex before calling any aci/hci functions */
#define BLE_WRAP_PREPROC() do{ \
                             if( __get_IPSR() != 0 )while(1); \
                             osMutexAcquire(LinkLayerMutex, osWaitForever); \
                           }while(0)

/* Release Link Layer Mutex */
/* Trigger BLE Host stack process after calling any aci/hci functions */
#define BLE_WRAP_POSTPROC() do{ \
                              osMutexRelease(LinkLayerMutex); \
                              BleStackCB_Process(); \
                            }while(0)

/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void BleStackCB_Process(void);
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 2 */

/* USER CODE END 2 */

#endif /* HOST_STACK_IF_H */
