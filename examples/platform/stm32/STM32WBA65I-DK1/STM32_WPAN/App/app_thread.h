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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef APP_THREAD_H
#define APP_THREAD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "thread.h"
/* Exported types ------------------------------------------------------------*/
/*
 *  List of all errors tracked by the Thread application.
 *  Some of these errors may be fatal or just warnings
 */
typedef enum
{
    ERR_THREAD_SET_STATE_CB,
    ERR_THREAD_SET_CHANNEL,
    ERR_THREAD_SET_PANID,
    ERR_THREAD_SET_THRESHOLD,
    ERR_THREAD_SET_MASTERKEY,
    ERR_THREAD_IPV6_ENABLE,
    ERR_THREAD_START,
    ERR_THREAD_ERASE_PERSISTENT_INFO,
    ERR_THREAD_SET_NETWORK_KEY,
    ERR_THREAD_LINK_MODE,
    ERR_THREAD_POLL_MODE,
    /* USER CODE BEGIN ERROR_APPLI_ENUM */
    ERR_THREAD_COAP_START,
    ERR_THREAD_COAP_ADD_RESSOURCE,
    ERR_THREAD_MESSAGE_READ,
    ERR_THREAD_COAP_SEND_RESPONSE,
    ERR_THREAD_COAP_NEW_MSG,
    ERR_THREAD_COAP_APPEND,
    ERR_THREAD_COAP_SEND_REQUEST,
    ERR_THREAD_COAP_DATA_RESPONSE,
    ERR_THREAD_SETUP,
    ERR_TIMER_INIT,
    ERR_TIMER_START,
    ERR_THREAD_MSG_COMPARE_FAILED,
    ERR_THREAD_COAP_ADDRESS_NOT_DEFINED,
    /* USER CODE END ERROR_APPLI_ENUM */
    ERR_THREAD_CHECK_WIRELESS
} ErrAppliIdEnum_t;

/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
#define APP_READ32_REG(base_addr) (*(volatile uint32_t *) (base_addr))
#define APP_WRITE32_REG(base_addr, data) (*(volatile uint32_t *) (base_addr) = (data))

#define OT_API_CALL(...)                                                                                                           \
    do                                                                                                                             \
    {                                                                                                                              \
        osMutexAcquire(LinkLayerMutex, osWaitForever);                                                                             \
        __VA_ARGS__;                                                                                                               \
        osMutexRelease(LinkLayerMutex);                                                                                            \
        osThreadFlagsSet(WpanTaskHandle, 1U << CFG_RTOS_FLAG_OT_Tasklet);                                                          \
    } while (0)

/* ipv6-addressing defines        */
/* Key Point: A major difference between FTDs and MTDs are that FTDs subscribe to the ff03::2 multicast address.
 * MTDs do not. */
#define MULTICAST_FTD_MED "ff03::1"
#define MULTICAST_FTD_BORDER_ROUTER "ff03::2"

/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported functions prototypes ---------------------------------------------*/
void Thread_Init(void);

void APP_THREAD_Init(void);
void APP_THREAD_ScheduleUART(void);
void APP_THREAD_Error(uint32_t ErrId, uint32_t ErrCode);
void APP_THREAD_ProcessAlarm(void * argument);
void APP_THREAD_ProcessUsAlarm(void * argument);
void APP_THREAD_ProcessOpenThreadTasklets(void * argument);
void APP_THREAD_LockThreadStack(void);
bool APP_THREAD_TryLockThreadStack(void);
void APP_THREAD_UnLockThreadStack(void);
otInstance * APP_THREAD_GetotInstance(void);

#if (OT_CLI_USE == 1)
void APP_THREAD_ProcessUart(void * argument);
#endif

/* USER CODE BEGIN EFP */

/* USER CODE BEGIN EFP */
#ifdef __cplusplus
}
#endif

#endif /* APP_THREAD_H */
