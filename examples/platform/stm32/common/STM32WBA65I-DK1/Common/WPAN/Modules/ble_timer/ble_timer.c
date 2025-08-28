/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ble_timer.c
  * @author  MCD Application Team
  * @brief   This module implements the timer core functions
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "stm32wbaxx.h"
#include "blestack.h"
#include "stm32_timer.h"
#include "bleplat.h"
#include "stm_list.h"
#include "ble_timer.h"
#include "advanced_memory_manager.h"
#include "app_conf.h"
#include "ll_sys.h"
#include "stm32_rtos.h"
#include "cmsis_os2.h"
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  tListNode           node;         /* Actual node in the list */
  uint16_t            id;           /* Id of the timer */
  UTIL_TIMER_Object_t timerObject;  /* Timer Server object */
}BLE_TIMER_t;

/* Private defines -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
tListNode BLE_TIMER_List;
static BLE_TIMER_t* BLE_TIMER_timer;

/* BLE_TIMER_TASK related resources */
osSemaphoreId_t     BleTimerSemaphore;
osThreadId_t        BleTimerThread;

/* Private functions prototype------------------------------------------------*/
void BLE_TIMER_Background(void);
static void BLE_TIMER_Callback(void* arg);
static BLE_TIMER_t* BLE_TIMER_GetFromList(tListNode * listHead, uint16_t id);
static void BleTimer_Task_Entry(void* thread_input);

void BLE_TIMER_Init(void)
{
  /* This function initializes the timer Queue */
  LST_init_head(&BLE_TIMER_List);

  /* Register Timer background task */
  const osSemaphoreAttr_t BleTimerSemaphore_attributes = {
    .name = "BLE timer Semaphore"
  };
  BleTimerSemaphore = osSemaphoreNew(1U, 0U, &BleTimerSemaphore_attributes);
  if (BleTimerSemaphore == NULL)
  {
    Error_Handler();
  }
  const osThreadAttr_t BleTimerTask_attributes = {
    .name = "BLE timer Task",
    .priority = (osPriority_t)CFG_TASK_PRIO_BLE_TIMER,
    .stack_size = TASK_BLE_TIMER_STACK_SIZE
  };

  BleTimerThread = osThreadNew(BleTimer_Task_Entry, NULL, &BleTimerTask_attributes);
  if (BleTimerThread == NULL)
  {
    Error_Handler();
  }

  /* Initialize the Timer Server */
  UTIL_TIMER_Init();
}

uint8_t BLE_TIMER_Start(uint16_t id, uint32_t timeout)
{
  /* If the timer's id already exists, stop it */
  BLE_TIMER_Stop(id);

  /* Create a new timer instance and add it to the list */
  BLE_TIMER_t *timer = NULL;

  if(AMM_ERROR_OK != AMM_Alloc (CFG_AMM_VIRTUAL_STACK_BLE,
                                DIVC(sizeof(BLE_TIMER_t), sizeof(uint32_t)),
                                (uint32_t **)&timer,
                                NULL))
  {
    return BLE_STATUS_INSUFFICIENT_RESOURCES;
  }

  timer->id = id;
  LST_insert_tail(&BLE_TIMER_List, (tListNode *)timer);

  if(UTIL_TIMER_Create(&timer->timerObject, timeout, UTIL_TIMER_ONESHOT, &BLE_TIMER_Callback, timer) != UTIL_TIMER_OK)
  {
    LST_remove_node ((tListNode *)timer);
    (void)AMM_Free((uint32_t *)timer);
    return BLE_STATUS_FAILED;
  }

  if(UTIL_TIMER_Start(&timer->timerObject) != UTIL_TIMER_OK)
  {
    LST_remove_node ((tListNode *)timer);
    (void)AMM_Free((uint32_t *)timer);
    return BLE_STATUS_FAILED;
  }

  return BLE_STATUS_SUCCESS;
}

void BLE_TIMER_Stop(uint16_t id)
{
  /* Search for the id in the timers list */
  BLE_TIMER_t* timer = BLE_TIMER_GetFromList(&BLE_TIMER_List, id);

  /* If the timer's id exists, stop it */
  if(NULL != timer)
  {
    UTIL_TIMER_Stop(&timer->timerObject);
    LST_remove_node((tListNode *)timer);

    (void)AMM_Free((uint32_t *)timer);
  }
}

void BLE_TIMER_Background(void)
{
  BLEPLATCB_TimerExpiry( (uint16_t)BLE_TIMER_timer->id);
  HostStack_Process( );

  /* Delete the BLE_TIMER_timer from the list */
  LST_remove_node((tListNode *)BLE_TIMER_timer);

  (void)AMM_Free((uint32_t *)BLE_TIMER_timer);
}

static void BleTimer_Task_Entry(void* thread_input)
{
  (void)(thread_input);
#if HIGHWATERMARK
    UBaseType_t uxHighWaterMark;
    UBaseType_t lastHighWaterMark = 0;
#endif // endif HIGHWATERMARK

  while(1)
  {
    osSemaphoreAcquire(BleTimerSemaphore, osWaitForever);
    BLE_TIMER_Background();
    osThreadYield();
#if HIGHWATERMARK
        uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
        if (uxHighWaterMark != lastHighWaterMark) {
            APP_DBG("\x1b[34m" "BleTimerTaskEntry_stack_HighWaterMark %lu \n" "\x1b[0m", uxHighWaterMark);
            lastHighWaterMark = uxHighWaterMark;
        }
#endif
  }
}

static void BLE_TIMER_Callback(void* arg)
{
  BLE_TIMER_timer = (BLE_TIMER_t*)arg;

  osSemaphoreRelease(BleTimerSemaphore);
}

static BLE_TIMER_t* BLE_TIMER_GetFromList(tListNode * listHead, uint16_t id)
{
  BLE_TIMER_t* currentNode = (BLE_TIMER_t*)listHead->next;
  while((tListNode *)currentNode != listHead)
  {
    if(currentNode->id == id)
    {
      return currentNode;
    }
    LST_get_next_node((tListNode *)currentNode, (tListNode **)&currentNode);
  }
  return NULL;
}

