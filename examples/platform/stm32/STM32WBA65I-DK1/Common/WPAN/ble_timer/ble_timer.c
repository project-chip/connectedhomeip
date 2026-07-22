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
#include "ble_timer.h"
#include "advanced_memory_manager.h"
#include "app_conf.h"
#include "bleplat.h"
#include "blestack.h"
#include "host_stack_if.h"
#include "ll_sys.h"
#include "log_module.h"
#include "main.h"
#include "stm32_rtos.h"
#include "stm32_timer.h"
#include "stm_list.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
    tListNode node;                  /* Actual node in the list */
    uint16_t id;                     /* Id of the timer */
    UTIL_TIMER_Object_t timerObject; /* Timer Server object */
} BLE_TIMER_t;

/* Private defines -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static tListNode BLE_TIMER_RunningList = { 0 };
static tListNode BLE_TIMER_ExpiredList = { 0 };

/* Private functions prototype------------------------------------------------*/
static void BLE_TIMER_Callback(void * arg);
static BLE_TIMER_t * BLE_TIMER_GetFromList(tListNode * listHead, uint16_t id);

void BLE_TIMER_Init(void)
{
    /* Initializes timers Queue */
    LST_init_head(&BLE_TIMER_RunningList);
    LST_init_head(&BLE_TIMER_ExpiredList);
}

void BLE_TIMER_Deinit(void)
{
    tListNode * listNodeRemoved;

    while (LST_is_empty(&BLE_TIMER_RunningList) != TRUE)
    {
        LST_remove_tail(&BLE_TIMER_RunningList, &listNodeRemoved);
        (void) AMM_Free((uint32_t *) listNodeRemoved);
    }
    while (LST_is_empty(&BLE_TIMER_ExpiredList) != TRUE)
    {
        LST_remove_tail(&BLE_TIMER_ExpiredList, &listNodeRemoved);
        (void) AMM_Free((uint32_t *) listNodeRemoved);
    }

    /* Reset timers Queues */
    LST_init_head(&BLE_TIMER_RunningList);
    LST_init_head(&BLE_TIMER_ExpiredList);
}

uint8_t BLE_TIMER_Start(uint16_t id, uint32_t timeout)
{
    BLE_TIMER_t * timer = NULL;

    /* If the timer's id already exists, stop it */
    BLE_TIMER_Stop(id);

    /* Create a new timer instance and add it to the list */
    if (AMM_ERROR_OK !=
        AMM_Alloc(CFG_AMM_VIRTUAL_BLE_TIMERS, DIVC(sizeof(BLE_TIMER_t), sizeof(uint32_t)), (uint32_t **) &timer, NULL))
    {
        return BLE_STATUS_INSUFFICIENT_RESOURCES;
    }

    if (UTIL_TIMER_Create(&timer->timerObject, timeout, UTIL_TIMER_ONESHOT, &BLE_TIMER_Callback, timer) != UTIL_TIMER_OK)
    {
        (void) AMM_Free((uint32_t *) timer);
        return BLE_STATUS_FAILED;
    }

    if (UTIL_TIMER_Start(&timer->timerObject) != UTIL_TIMER_OK)
    {
        (void) AMM_Free((uint32_t *) timer);
        return BLE_STATUS_FAILED;
    }

    timer->id = id;
    LST_insert_tail(&BLE_TIMER_RunningList, (tListNode *) timer);

    return BLE_STATUS_SUCCESS;
}

void BLE_TIMER_Stop(uint16_t id)
{
    BLE_TIMER_t * timer;

    /* Search for the id in the running timers list */
    timer = BLE_TIMER_GetFromList(&BLE_TIMER_RunningList, id);
    /* If not found, try elapsed timers list */
    if (NULL == timer)
    {
        timer = BLE_TIMER_GetFromList(&BLE_TIMER_ExpiredList, id);
    }

    /* If the timer's id exists, stop it */
    if (NULL != timer)
    {
        UTIL_TIMER_Stop(&timer->timerObject);
        LST_remove_node((tListNode *) timer);

        (void) AMM_Free((uint32_t *) timer);
    }
}

void BLE_TIMER_Background(void)
{
    BLE_TIMER_t * timer;

    if (TRUE != LST_is_empty(&BLE_TIMER_ExpiredList))
    {
        /* Get first timer from (sorted) expired list and remove it from this list */
        LST_remove_head(&BLE_TIMER_ExpiredList, (tListNode **) &timer);
        BLEPLATCB_TimerExpiry(timer->id);
        BleStackCB_Process();
        (void) AMM_Free((uint32_t *) timer);

        if (TRUE != LST_is_empty(&BLE_TIMER_ExpiredList))
        {
            /* At least one other timer expired and has not been processed,
               so trigger task again */
            osThreadFlagsSet(WpanTaskHandle, 1U << CFG_RTOS_FLAG_BLEtimer);
        }
    }
}

static void BLE_TIMER_Callback(void * arg)
{
    /* Remove timer from running list */
    LST_remove_node((tListNode *) arg);
    /* Add it to expired list */
    LST_insert_tail(&BLE_TIMER_ExpiredList, (tListNode *) arg);

    osThreadFlagsSet(WpanTaskHandle, 1U << CFG_RTOS_FLAG_BLEtimer);
}

static BLE_TIMER_t * BLE_TIMER_GetFromList(tListNode * listHead, uint16_t id)
{
    BLE_TIMER_t * currentNode;

    LST_get_next_node(listHead, (tListNode **) &currentNode);
    while ((tListNode *) currentNode != listHead)
    {
        if (currentNode->id == id)
        {
            return currentNode;
        }
        LST_get_next_node((tListNode *) currentNode, (tListNode **) &currentNode);
    }
    return NULL;
}
