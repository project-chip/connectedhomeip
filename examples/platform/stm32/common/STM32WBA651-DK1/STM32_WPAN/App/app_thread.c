/* USER CODE BEGIN Header */
/**
 ******************************************************************************
  * File Name          : app_thread.c
  * Description        : Thread Application.
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

/* Includes ------------------------------------------------------------------*/
#include <assert.h>
#include <stdint.h>

#include "app_conf.h"
#include "app_common.h"
#include "app_entry.h"
#include "app_thread.h"
#include "dbg_trace.h"
#include "stm32_rtos.h"
#include "stm32_timer.h"
#include "common_types.h"
#include "instance.h"
#include "cli.h"
#include "radio.h"
#include "platform.h"
#include "tasklet.h"
#include "ll_sys_startup.h"
#include "event_manager.h"
#include "platform_wba.h"
#include "link.h"
#include "thread.h"

#include "joiner.h"
#include OPENTHREAD_CONFIG_FILE

/* Private includes -----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32wba65i_discovery.h"
#include "data_transfer.h"
#include "udp.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private defines -----------------------------------------------------------*/
#define C_SIZE_CMD_STRING       256U
#define C_PANID                 0xBA98U
#define C_CHANNEL_NB            16U
#define C_CCA_THRESHOLD         -70

/* USER CODE BEGIN PD */
#define C_RESSOURCE                     "light"



/* USER CODE END PD */

/* Private macros ------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private function prototypes -----------------------------------------------*/
static void APP_THREAD_DeviceConfig(void);
static void APP_THREAD_TraceError(const char * pMess, uint32_t ErrCode);

#if (OT_CLI_USE == 1)
static void APP_THREAD_CliInit(otInstance *aInstance);
static void APP_THREAD_ProcessUart(void);
#endif /* OT_CLI_USE */

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private variables -----------------------------------------------*/
otInstance * PtOpenThreadInstance;

/* USER CODE BEGIN PV */



const osThreadAttr_t stAlarmTaskAttributes = 
{
  .name = "Alarm Task",
  .priority = CFG_TASK_PRIO_ALARM,
  .stack_size = TASK_ALARM_STACK_SIZE
};

const osThreadAttr_t stUsAlarmTaskAttributes = 
{
  .name = "UsAlarm Task",
  .priority = CFG_TASK_PRIO_US_ALARM,
  .stack_size = TASK_ALARM_US_STACK_SIZE
};

const osThreadAttr_t stTaskletsTaskAttributes = 
{
  .name = "Tasklets Task",
  .priority = CFG_TASK_PRIO_TASKLETS,
  .stack_size = TASK_TASKLETS_STACK_SIZE
};


const osThreadAttr_t stCliUartTaskAttributes = 
{
  .name = "CliUart Task",
  .priority = CFG_TASK_PRIO_CLI_UART,
  .stack_size = TASK_CLI_UART_STACK_SIZE
};

osSemaphoreId_t       AlarmSemaphore, UsAlarmSemaphore;
osThreadId_t          AlarmTaskId, UsAlarmTaskId;

osSemaphoreId_t       CliUartSemaphore, TaskletsSemaphore, TaskletsMutex;
os_thread_id          CliUartTaskId, TaskletsTaskId, SendCoapMsgTaskId;

/* USER CODE END PV */

/* Functions Definition ------------------------------------------------------*/

void APP_THREAD_ScheduleAlarm(void)
{
  osSemaphoreRelease(AlarmSemaphore);
}

void APP_THREAD_ScheduleUsAlarm(void)
{
  osSemaphoreRelease(UsAlarmSemaphore);
}

void APP_THREAD_LockThreadStack(void)
{
	osSemaphoreAcquire(TaskletsMutex, osWaitForever);
}

bool APP_THREAD_TryLockThreadStack(void)
{
	return osSemaphoreAcquire(TaskletsMutex, 0) == osOK;
}

void APP_THREAD_UnLockThreadStack(void)
{
  osSemaphoreRelease(TaskletsMutex);
}

otInstance*  APP_THREAD_GetotInstance(void)
{
	return PtOpenThreadInstance;
}
/**
 * @brief   Background Task for Alarm.
 */
void Alarm_Task( void * argument )
{
  UNUSED(p_param);  
#if HIGHWATERMARK
    UBaseType_t uxHighWaterMark;
    UBaseType_t lastHighWaterMark = 0;
#endif // endif HIGHWATERMARK

  while(1)
  {
    /* Wait for task semaphore to be released */
    osSemaphoreAcquire( AlarmSemaphore, osWaitForever );
    ProcessAlarm();
    osThreadYield();
#if HIGHWATERMARK
        uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
        if (uxHighWaterMark != lastHighWaterMark) {
            APP_DBG("\x1b[34m" "AlarmTask_stack_HighWaterMark %lu \n" "\x1b[0m", uxHighWaterMark);
            lastHighWaterMark = uxHighWaterMark;
        }
#endif
  }
}

/**
 * @brief   Background Task for US Alarm.
 */
void Us_Alarm_Task( void * argument )
{
  UNUSED(p_param);
#if HIGHWATERMARK
    UBaseType_t uxHighWaterMark;
    UBaseType_t lastHighWaterMark = 0;
#endif // endif HIGHWATERMARK

  while(1)
  {
    /* Wait for task semaphore to be released */
    osSemaphoreAcquire( UsAlarmSemaphore, osWaitForever );
    ProcessUsAlarm();
    osThreadYield();
#if HIGHWATERMARK
        uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
        if (uxHighWaterMark != lastHighWaterMark) {
            APP_DBG("\x1b[34m" "UsAlarmTask_stack_HighWaterMark %lu \n" "\x1b[0m", uxHighWaterMark);
            lastHighWaterMark = uxHighWaterMark;
        }
#endif
  }
}


#if (OT_CLI_USE == 1)
/**
 * @brief   Background Task for Cli Uart.
 */
void Cli_Uart_Task( void * argument )
{
  UNUSED(p_param);
#if HIGHWATERMARK
    UBaseType_t uxHighWaterMark;
    UBaseType_t lastHighWaterMark = 0
#endif // endif HIGHWATERMARK

  while(1)
  {
    /* Wait for task semaphore to be released */
    osSemaphoreAcquire( CliUartSemaphore, osWaitForever );
    APP_THREAD_ProcessUart();
    osThreadYield();
#if HIGHWATERMARK
        uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
        if (uxHighWaterMark != lastHighWaterMark) {
            APP_DBG("\x1b[34m" "CliUartTask_stack_HighWaterMark %lu \n" "\x1b[0m", uxHighWaterMark);
            lastHighWaterMark = uxHighWaterMark;
        }
#endif
  }
}
#endif // (OT_CLI_USE == 1)

/**
 * @brief   Background Task for Tasklets.
 */
void Tasklets_Task( void * argument )
{
  UNUSED(p_param);  
#if HIGHWATERMARK
    UBaseType_t uxHighWaterMark;
    UBaseType_t lastHighWaterMark = 0;
#endif // endif HIGHWATERMARK

    while(1)
      {
        /* Wait for task semaphore to be released */
        osSemaphoreAcquire( TaskletsSemaphore, osWaitForever );
        APP_THREAD_LockThreadStack();
        osMutexAcquire(LinkLayerMutex, osWaitForever);
        ProcessOpenThreadTasklets();
        APP_THREAD_UnLockThreadStack();
        osMutexRelease(LinkLayerMutex);
        osThreadYield();
    #if HIGHWATERMARK
            uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
            if (uxHighWaterMark != lastHighWaterMark) {
                APP_DBG("\x1b[34m" "TaskletsTask_stack_HighWaterMark %lu \n" "\x1b[0m", uxHighWaterMark);
                lastHighWaterMark = uxHighWaterMark;
            }
    #endif
  }
}


void Thread_Init(void)
{
#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
  size_t otInstanceBufferLength = 0;
  uint8_t *otInstanceBuffer = NULL;
#endif

  otSysInit(0, NULL);

#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
  // Call to query the buffer size
  (void)otInstanceInit(NULL, &otInstanceBufferLength);

  // Call to allocate the buffer
  otInstanceBuffer = (uint8_t *)malloc(otInstanceBufferLength);
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

  /* Register tasks */
#if (OT_CLI_USE == 1)
  /* Create thread for cli uart and semaphore to control it*/
  CliUartSemaphore = osSemaphoreNew( 1, 0, NULL );
  if ( CliUartSemaphore == NULL )
  { 
    APP_DBG( "ERROR FREERTOS : CLI UART SEMAPHORE CREATION FAILED" );
    while(1);
  }  

  CliUartTaskId = osThreadNew( Cli_Uart_Task, NULL, &stCliUartTaskAttributes );
  if ( CliUartTaskId == NULL )
  { 
    APP_DBG( "ERROR FREERTOS : CLI UART TASK CREATION FAILED" );
    while(1);
  }  
#endif

  /* Create thread for Alarm and semaphore to control it*/
  AlarmSemaphore = osSemaphoreNew( 1, 0, NULL );
  if ( AlarmSemaphore == NULL )
  { 
    APP_DBG( "ERROR FREERTOS : ALARM SEMAPHORE CREATION FAILED" );
    while(1);
  }  

  AlarmTaskId = osThreadNew( Alarm_Task, NULL, &stAlarmTaskAttributes );
  if ( AlarmTaskId == NULL )
  { 
    APP_DBG( "ERROR FREERTOS : ALARM TASK CREATION FAILED" );
    while(1);
  }  
  
  /* Create thread for US Alarm and semaphore to control it*/
  UsAlarmSemaphore = osSemaphoreNew( 1, 1, NULL );
  if ( UsAlarmSemaphore == NULL )
  { 
    APP_DBG( "ERROR FREERTOS : US ALARM SEMAPHORE CREATION FAILED" );
    while(1);
  }  

  UsAlarmTaskId = osThreadNew( Us_Alarm_Task, NULL, &stUsAlarmTaskAttributes );
  if ( UsAlarmTaskId == NULL )
  { 
    APP_DBG( "ERROR FREERTOS : US ALARM TASK CREATION FAILED" );
    while(1);
  } 

  /* Create thread for Tasklets and semaphore to control it*/
  TaskletsSemaphore = osSemaphoreNew( 1, 1, NULL );
  if ( TaskletsSemaphore == NULL )
  { 
    APP_DBG( "ERROR FREERTOS : TASKLETS SEMAPHORE CREATION FAILED" );
    while(1);
  }  

  /* Create Mutex to protect OT thread */
  TaskletsMutex = osSemaphoreNew( 1, 1, NULL );
  if ( TaskletsMutex == NULL )
  {
    APP_DBG( "ERROR FREERTOS : TASKLETS SEMAPHORE CREATION FAILED" );
    while(1);
  }

  TaskletsTaskId = osThreadNew( Tasklets_Task, NULL, &stTaskletsTaskAttributes );
  if ( TaskletsTaskId == NULL )
  { 
    APP_DBG( "ERROR FREERTOS : TASKLETS TASK CREATION FAILED" );
    while(1);
  } 

  ll_sys_thread_init();



#if (OT_CLI_USE == 1)
  osSemaphoreRelease(CliUartSemaphore);
#endif

  /* USER CODE BEGIN INIT TASKS */

  /* USER CODE END INIT TASKS */


}


void ProcessAlarm(void)
{
  arcAlarmProcess(PtOpenThreadInstance);
}

void ProcessUsAlarm(void)
{
  arcUsAlarmProcess(PtOpenThreadInstance);
}

void ProcessTasklets(void)
{
  if (otTaskletsArePending(PtOpenThreadInstance) == TRUE)
  {
    osSemaphoreRelease(TaskletsSemaphore);
  }
}

/**
 * @brief  ProcessOpenThreadTasklets.
 * @param  None
 * @param  None
 * @retval None
 */
void ProcessOpenThreadTasklets(void)
{
  /* wakeUp the system */
 // ll_sys_radio_hclk_ctrl_req(LL_SYS_RADIO_HCLK_LL_BG, LL_SYS_RADIO_HCLK_ON);
 // ll_sys_dp_slp_exit();

  /* process the tasklet */
  otTaskletsProcess(PtOpenThreadInstance);

  /* Put the IP802_15_4 back to sleep mode */
 // ll_sys_radio_hclk_ctrl_req(LL_SYS_RADIO_HCLK_LL_BG, LL_SYS_RADIO_HCLK_OFF);

  /* Reschedule the tasklets if any */
  ProcessTasklets();
}

/**
 * OpenThread calls this function when the tasklet queue transitions from empty to non-empty.
 *
 * @param[in] aInstance A pointer to an OpenThread instance.
 */
void otTaskletsSignalPending(otInstance *aInstance)
{
  osSemaphoreRelease(TaskletsSemaphore);
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
    APP_THREAD_Error(ERR_THREAD_SET_THRESHOLD,error);
  }


  otPlatRadioEnableSrcMatch(PtOpenThreadInstance, true);


  /* USER CODE BEGIN DEVICECONFIG */

  /* USER CODE END DEVICECONFIG */
}

void APP_THREAD_Init( void )
{
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

  while(1U == 1U)
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

  switch(ErrId)
  {
    case ERR_THREAD_SET_STATE_CB :
        APP_THREAD_TraceError("ERROR : ERR_THREAD_SET_STATE_CB ",ErrCode);
        break;

    case ERR_THREAD_SET_CHANNEL :
        APP_THREAD_TraceError("ERROR : ERR_THREAD_SET_CHANNEL ",ErrCode);
        break;

    case ERR_THREAD_SET_PANID :
        APP_THREAD_TraceError("ERROR : ERR_THREAD_SET_PANID ",ErrCode);
        break;

    case ERR_THREAD_IPV6_ENABLE :
        APP_THREAD_TraceError("ERROR : ERR_THREAD_IPV6_ENABLE ",ErrCode);
        break;

    case ERR_THREAD_START :
        APP_THREAD_TraceError("ERROR: ERR_THREAD_START ", ErrCode);
        break;

    case ERR_THREAD_ERASE_PERSISTENT_INFO :
        APP_THREAD_TraceError("ERROR : ERR_THREAD_ERASE_PERSISTENT_INFO ",ErrCode);
        break;

    case ERR_THREAD_SET_NETWORK_KEY :
        APP_THREAD_TraceError("ERROR : ERR_THREAD_SET_NETWORK_KEY ",ErrCode);
        break;

    case ERR_THREAD_CHECK_WIRELESS :
        APP_THREAD_TraceError("ERROR : ERR_THREAD_CHECK_WIRELESS ",ErrCode);
        break;

    /* USER CODE BEGIN APP_THREAD_Error_2 */
    case ERR_THREAD_COAP_START :
        APP_THREAD_TraceError("ERROR : ERR_THREAD_COAP_START ",ErrCode);
        break;

    case ERR_THREAD_COAP_ADD_RESSOURCE :
        APP_THREAD_TraceError("ERROR : ERR_THREAD_COAP_ADD_RESSOURCE ",ErrCode);
        break;

    case ERR_THREAD_MESSAGE_READ :
        APP_THREAD_TraceError("ERROR : ERR_THREAD_MESSAGE_READ ",ErrCode);
        break;

    case ERR_THREAD_COAP_SEND_RESPONSE :
        APP_THREAD_TraceError("ERROR : ERR_THREAD_COAP_SEND_RESPONSE ",ErrCode);
        break;

    case ERR_THREAD_COAP_APPEND :
        APP_THREAD_TraceError("ERROR : ERR_THREAD_COAP_APPEND ",ErrCode);
        break;

    case ERR_THREAD_COAP_SEND_REQUEST :
        APP_THREAD_TraceError("ERROR : ERR_THREAD_COAP_SEND_REQUEST ",ErrCode);
        break;

    case ERR_THREAD_MSG_COMPARE_FAILED:
        APP_THREAD_TraceError("ERROR : ERR_THREAD_MSG_COMPARE_FAILED ",ErrCode);
        break;

    /* USER CODE END APP_THREAD_Error_2 */
    default :
        APP_THREAD_TraceError("ERROR Unknown ", 0);
        break;
  }
}



void app_logger_write(uint8_t *buffer, uint32_t size)
{
  //UTIL_ADV_TRACE_COND_Send(VLEVEL_ALWAYS, ~0x0, 0, buffer, (uint16_t)size);
}

#if (OT_CLI_USE == 1)
/* OT CLI UART functions */
static void APP_THREAD_ProcessUart(void)
{
  arcUartProcess();
}

void APP_THREAD_ScheduleUART(void)
{
  osSemaphoreRelease(CliUartSemaphore);
}

static void APP_THREAD_CliInit(otInstance *aInstance)
{

  otPlatUartEnable();
  otCliInit(aInstance, CliUartOutput, aInstance);
}
#endif /* OT_CLI_USE */

/* USER CODE BEGIN FD_LOCAL_FUNCTIONS */

/* USER CODE END FD_LOCAL_FUNCTIONS */

