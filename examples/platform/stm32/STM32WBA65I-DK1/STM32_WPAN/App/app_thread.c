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
#include <assert.h>
#include <stdint.h>

#include "app_common.h"
#include "app_conf.h"
#include "app_entry.h"
#include "app_thread.h"
#include "dbg_trace.h"
#include "log_module.h"
#include "stm32_rtos.h"
#include "stm32_timer.h"
#if (CFG_LPM_LEVEL != 0)
#include "stm32_lpm.h"
#endif // CFG_LPM_LEVEL
#include "cli.h"
#include "coap.h"
#include "common_types.h"
#include "event_manager.h"
#include "instance.h"
#include "link.h"
#include "ll_sys_startup.h"
#include "platform.h"
#include "platform_wba.h"
#include "radio.h"
#include "tasklet.h"
#include "thread.h"
#if (OT_CLI_USE == 1)
#include "uart.h"
#endif
#include "alarm.h"
#include "joiner.h"
#include OPENTHREAD_CONFIG_FILE
#include "stm32_lpm_if.h"
/* Private includes -----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_bsp.h"
#include "data_transfer.h"
#include "stm32wba65i_discovery.h"
#include "udp.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private defines -----------------------------------------------------------*/
#define C_SIZE_CMD_STRING 256U
#define C_PANID 0xBA98U
#define C_CHANNEL_NB 16U
#define C_CCA_THRESHOLD -70

/* USER CODE BEGIN PD */
#define C_RESSOURCE "light"

/* USER CODE END PD */

/* Private macros ------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private function prototypes -----------------------------------------------*/
static void APP_THREAD_DeviceConfig(void);
static void APP_THREAD_TraceError(const char * pMess, uint32_t ErrCode);

#if (OT_CLI_USE == 1)
static void APP_THREAD_CliInit(otInstance * aInstance);
static void APP_THREAD_ProcessUart(void);
#endif /* OT_CLI_USE */

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private variables -----------------------------------------------*/
otInstance * PtOpenThreadInstance;

/* USER CODE BEGIN PV */

osSemaphoreId_t TaskletsMutex;

/* USER CODE END PV */

/* Functions Definition ------------------------------------------------------*/

/**
 * @brief  APP_THREAD_ProcessAlarm.
 * @param  ULONG lArgument (unused)
 * @param  None
 * @retval None
 */
void APP_THREAD_ProcessAlarm(void * argument)
{
    UNUSED(argument);

    arcAlarmProcess(PtOpenThreadInstance);
}

/**
 * @brief  APP_THREAD_ProcessUsAlarm.
 * @param  ULONG lArgument (unused)
 * @param  None
 * @retval None
 */
void APP_THREAD_ProcessUsAlarm(void * argument)
{
    UNUSED(argument);

    arcUsAlarmProcess(PtOpenThreadInstance);
}

/**
 * @brief  APP_THREAD_ProcessOpenThreadTasklets.
 * @param  ULONG lArgument (unused)
 * @param  None
 * @retval None
 */
void APP_THREAD_ProcessOpenThreadTasklets(void * argument)
{
    UNUSED(argument);

    /* process the tasklet */
    otTaskletsProcess(PtOpenThreadInstance);
}

/**
 * OpenThread calls this function when the tasklet queue transitions from empty to non-empty.
 *
 * @param[in] aInstance A pointer to an OpenThread instance.
 */
void otTaskletsSignalPending(otInstance * aInstance)
{
    UNUSED(aInstance);

    osThreadFlagsSet(WpanTaskHandle, 1U << CFG_RTOS_FLAG_OT_Tasklet);
}

/**
 * @brief  APP_THREAD_ScheduleAlarm.
 * @param  None
 * @param  None
 * @retval None
 */
void APP_THREAD_ScheduleAlarm(void)
{
    osThreadFlagsSet(WpanTaskHandle, 1U << CFG_RTOS_FLAG_OT_Alarm_ms);
}

/**
 * @brief  APP_THREAD_ScheduleUsAlarm.
 * @param  None
 * @param  None
 * @retval None
 */
void APP_THREAD_ScheduleUsAlarm(void)
{
    osThreadFlagsSet(WpanTaskHandle, 1U << CFG_RTOS_FLAG_OT_Alarm_us);
}

/**
 * @brief  APP_THREAD_LockThreadStack.
 * @param  None
 * @param  None
 * @retval None
 */
void APP_THREAD_LockThreadStack(void)
{
    osSemaphoreAcquire(TaskletsMutex, osWaitForever);
}

/**
 * @brief  APP_THREAD_TryLockThreadStack.
 * @param  None
 * @param  None
 * @retval bool
 */
bool APP_THREAD_TryLockThreadStack(void)
{
    return osSemaphoreAcquire(TaskletsMutex, 0) == osOK;
}

/**
 * @brief  APP_THREAD_UnLockThreadStack.
 * @param  None
 * @param  None
 * @retval None
 */
void APP_THREAD_UnLockThreadStack(void)
{
    osSemaphoreRelease(TaskletsMutex);
}

/**
 * @brief  APP_THREAD_GetotInstance.
 * @param  None
 * @param  None
 * @retval ot instance
 */
otInstance * APP_THREAD_GetotInstance(void)
{
    return PtOpenThreadInstance;
}

void Thread_Init(void)
{
#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    size_t otInstanceBufferLength = 0;
    uint8_t * otInstanceBuffer    = NULL;
#endif

    otSysInit(0, NULL);

#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    // Call to query the buffer size
    (void) otInstanceInit(NULL, &otInstanceBufferLength);

    // Call to allocate the buffer
    otInstanceBuffer = (uint8_t *) malloc(otInstanceBufferLength);
    assert(otInstanceBuffer);

    // Initialize OpenThread with the buffer
    PtOpenThreadInstance = otInstanceInit(otInstanceBuffer, &otInstanceBufferLength);
#else
    PtOpenThreadInstance = otInstanceInitSingle();
#endif

    assert(PtOpenThreadInstance);

#if (OT_CLI_USE == 1)
    APP_THREAD_CliInit(PtOpenThreadInstance);
#endif

    otDispatch_tbl_init(PtOpenThreadInstance);

    /* Create Mutex to protect OT thread */
    TaskletsMutex = osSemaphoreNew(1, 1, NULL);
    if (TaskletsMutex == NULL)
    {
        LOG_ERROR_APP("ERROR FREERTOS : TASKLETS SEMAPHORE CREATION FAILED");
        while (1)
            ;
    }

    ll_sys_thread_init();

    /* USER CODE BEGIN INIT TASKS */

    /* USER CODE END INIT TASKS */
}

/**
 * @brief Thread initialization.
 * @param  None
 * @retval None
 */
static void APP_THREAD_DeviceConfig(void)
{
    otError error = OT_ERROR_NONE;

    error = otPlatRadioSetCcaEnergyDetectThreshold(PtOpenThreadInstance, C_CCA_THRESHOLD);
    if (error != OT_ERROR_NONE)
    {
        APP_THREAD_Error(ERR_THREAD_SET_THRESHOLD, error);
    }

    otPlatRadioEnableSrcMatch(PtOpenThreadInstance, true);

#if (CFG_LPM_LEVEL == 2)
    UTIL_LPM_SetMaxMode(1 << CFG_LPM_APP, UTIL_LPM_MAX_MODE);
#endif // CFG_LPM_LEVEL
#if (CFG_LPM_LEVEL == 1)
    UTIL_LPM_SetMaxMode(1 << CFG_LPM_APP, UTIL_LPM_STOP1_MODE);
#endif // CFG_LPM_LEVEL

    /* USER CODE END DEVICECONFIG */
    /* USER CODE END DEVICECONFIG */
}

void APP_THREAD_Init(void)
{
#if (CFG_LPM_LEVEL != 0)
    UTIL_LPM_SetMaxMode(1 << CFG_LPM_APP, UTIL_LPM_SLEEP_MODE);
#endif // CFG_LPM_LEVEL

    Thread_Init();

    APP_THREAD_DeviceConfig();
}

/**
 * @brief  Warn the user that an error has occurred.
 *
 * @param  pMess  : Message associated to the error.
 * @param  ErrCode: Error code associated to the module (OpenThread or other module if any)
 * @retval None
 */
static void APP_THREAD_TraceError(const char * pMess, uint32_t ErrCode)
{
    /* USER CODE BEGIN TRACE_ERROR */
    LOG_ERROR_APP("**** FATAL ERROR = %s (Err = %d)", pMess, ErrCode);
    /* In this case, the LEDs on the Board will start blinking. */

    while (1U == 1U)
    {
    }

    /* USER CODE END TRACE_ERROR */
}

/**
 * @brief  Trace the error or the warning reported.
 * @param  ErrId :
 * @param  ErrCode
 * @retval None
 */
void APP_THREAD_Error(uint32_t ErrId, uint32_t ErrCode)
{
    /* USER CODE BEGIN APP_THREAD_Error_1 */

    /* USER CODE END APP_THREAD_Error_1 */

    switch (ErrId)
    {
    case ERR_THREAD_SET_STATE_CB:
        APP_THREAD_TraceError("ERROR : ERR_THREAD_SET_STATE_CB ", ErrCode);
        break;

    case ERR_THREAD_SET_CHANNEL:
        APP_THREAD_TraceError("ERROR : ERR_THREAD_SET_CHANNEL ", ErrCode);
        break;

    case ERR_THREAD_SET_PANID:
        APP_THREAD_TraceError("ERROR : ERR_THREAD_SET_PANID ", ErrCode);
        break;

    case ERR_THREAD_IPV6_ENABLE:
        APP_THREAD_TraceError("ERROR : ERR_THREAD_IPV6_ENABLE ", ErrCode);
        break;

    case ERR_THREAD_START:
        APP_THREAD_TraceError("ERROR: ERR_THREAD_START ", ErrCode);
        break;

    case ERR_THREAD_ERASE_PERSISTENT_INFO:
        APP_THREAD_TraceError("ERROR : ERR_THREAD_ERASE_PERSISTENT_INFO ", ErrCode);
        break;

    case ERR_THREAD_SET_NETWORK_KEY:
        APP_THREAD_TraceError("ERROR : ERR_THREAD_SET_NETWORK_KEY ", ErrCode);
        break;

    case ERR_THREAD_CHECK_WIRELESS:
        APP_THREAD_TraceError("ERROR : ERR_THREAD_CHECK_WIRELESS ", ErrCode);
        break;

    /* USER CODE BEGIN APP_THREAD_Error_2 */
    case ERR_THREAD_COAP_START:
        APP_THREAD_TraceError("ERROR : ERR_THREAD_COAP_START ", ErrCode);
        break;

    case ERR_THREAD_COAP_ADD_RESSOURCE:
        APP_THREAD_TraceError("ERROR : ERR_THREAD_COAP_ADD_RESSOURCE ", ErrCode);
        break;

    case ERR_THREAD_MESSAGE_READ:
        APP_THREAD_TraceError("ERROR : ERR_THREAD_MESSAGE_READ ", ErrCode);
        break;

    case ERR_THREAD_COAP_SEND_RESPONSE:
        APP_THREAD_TraceError("ERROR : ERR_THREAD_COAP_SEND_RESPONSE ", ErrCode);
        break;

    case ERR_THREAD_COAP_APPEND:
        APP_THREAD_TraceError("ERROR : ERR_THREAD_COAP_APPEND ", ErrCode);
        break;

    case ERR_THREAD_COAP_SEND_REQUEST:
        APP_THREAD_TraceError("ERROR : ERR_THREAD_COAP_SEND_REQUEST ", ErrCode);
        break;

    case ERR_THREAD_MSG_COMPARE_FAILED:
        APP_THREAD_TraceError("ERROR : ERR_THREAD_MSG_COMPARE_FAILED ", ErrCode);
        break;

    /* USER CODE END APP_THREAD_Error_2 */
    default:
        APP_THREAD_TraceError("ERROR Unknown ", 0);
        break;
    }
}

#if (OT_CLI_USE == 1)
/* OT CLI UART functions */
void APP_THREAD_ProcessUart(void * argument)
{
    UNUSED(argument);

    arcUartProcess();
}

void APP_THREAD_ScheduleUART(void)
{
    osThreadFlagsSet(WpanTaskHandle, 1U << CFG_RTOS_FLAG_OT_CLIuart);
}

static void APP_THREAD_CliInit(otInstance * aInstance)
{

    (void) otPlatUartEnable();
    otCliInit(aInstance, CliUartOutput, aInstance);
}
#endif /* OT_CLI_USE */

/* USER CODE BEGIN FD_LOCAL_FUNCTIONS */

/* USER CODE END FD_LOCAL_FUNCTIONS */
