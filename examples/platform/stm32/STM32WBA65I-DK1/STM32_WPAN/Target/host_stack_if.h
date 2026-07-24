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
#define BLE_WRAP_PREPROC()                                                                                                         \
    do                                                                                                                             \
    {                                                                                                                              \
        if (__get_IPSR() != 0)                                                                                                     \
            while (1)                                                                                                              \
                ;                                                                                                                  \
        osMutexAcquire(LinkLayerMutex, osWaitForever);                                                                             \
    } while (0)

/* Release Link Layer Mutex */
/* Trigger BLE Host stack process after calling any aci/hci functions */
#define BLE_WRAP_POSTPROC()                                                                                                        \
    do                                                                                                                             \
    {                                                                                                                              \
        osMutexRelease(LinkLayerMutex);                                                                                            \
        BleStackCB_Process();                                                                                                      \
    } while (0)

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
