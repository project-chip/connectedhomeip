/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Google LLC.
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

#include "AppConfig.h"
#include <em_device.h>
#include <platform/EFR32/freertos_bluetooth.h>
#include <stdint.h>
#include <string.h>

#include "sl_bt_api.h"
#include "sl_bt_ncp_host.h"
#include "sl_status.h"

#ifdef CONFIGURATION_HEADER
#include CONFIGURATION_HEADER
#endif // CONFIGURATION_HEADER

void BluetoothUpdate();
static sl_bt_msg_t ble_evt;
volatile sl_bt_msg_t * bluetooth_evt = &ble_evt;
SemaphoreHandle_t BluetoothMutex     = NULL;

static volatile uint32_t command_header;
static volatile void * command_data;
static volatile sl_bgapi_handler command_handler_func = NULL;

// Bluetooth task
#ifndef BLUETOOTH_STACK_SIZE
#define BLUETOOTH_STACK_SIZE (1024)
#endif
static void BluetoothTask(void * p_arg);
static TaskHandle_t BluetoothTaskHandle = NULL;

StackType_t bluetoothStack[BLUETOOTH_STACK_SIZE / sizeof(StackType_t)];
StaticTask_t bluetoothTaskStruct;

void sli_bt_cmd_handler_rtos_delegate(uint32_t header, sl_bgapi_handler handler, const void * payload);
extern void sli_bgapi_cmd_handler_delegate(uint32_t header, sl_bgapi_handler, const void *);
extern uint32_t sli_bt_can_sleep_ticks();

// Linklayer task
#ifndef LINKLAYER_STACK_SIZE
#define LINKLAYER_STACK_SIZE (1024)
#endif
static void LinklayerTask(void * p_arg);
static TaskHandle_t LinklayerTaskHandle = NULL;
StackType_t linkLayerStack[LINKLAYER_STACK_SIZE / sizeof(StackType_t)];
StaticTask_t linkLayerTaskStruct;
StaticSemaphore_t bluetoothMutexStruct;
//
#define RTOS_TICK_HZ 1024
#define BLUETOOTH_TICK_HZ 32768
#define BLUETOOTH_TO_RTOS_TICK (BLUETOOTH_TICK_HZ / RTOS_TICK_HZ)

EventGroupHandle_t bluetooth_event_flags;

sl_status_t bluetooth_start(UBaseType_t ll_priority, UBaseType_t stack_priority,
                            bluetooth_stack_init_func initialize_bluetooth_stack)
{
    sl_status_t err;
    bluetooth_event_flags = xEventGroupCreate();
    configASSERT(bluetooth_event_flags);

    BluetoothMutex = xSemaphoreCreateMutexStatic(&bluetoothMutexStruct);

    err = initialize_bluetooth_stack();

    if (err == SL_STATUS_OK)
    {
        // create tasks for Bluetooth host stack
        BluetoothTaskHandle =
            xTaskCreateStatic(BluetoothTask,                              /* Function that implements the task. */
                              BLE_STACK_TASK_NAME,                        /* Text name for the task. */
                              BLUETOOTH_STACK_SIZE / sizeof(StackType_t), /* Number of indexes in the xStack array. */
                              NULL,                                       /* Parameter passed into the task. */
                              stack_priority,                             /* Priority at which the task is created. */
                              bluetoothStack,                             /* Pointer to task heap */
                              &bluetoothTaskStruct);                      /* Variable that holds the task struct */

        // create tasks for Linklayer
        LinklayerTaskHandle =
            xTaskCreateStatic(LinklayerTask,                              /* Function that implements the task. */
                              BLE_LINK_TASK_NAME,                         /* Text name for the task. */
                              LINKLAYER_STACK_SIZE / sizeof(StackType_t), /* Number of indexes in the xStack array. */
                              NULL,                                       /* Parameter passed into the task. */
                              ll_priority,                                /* Priority at which the task is created. */
                              linkLayerStack,                             /* Pointer to task heap */
                              &linkLayerTaskStruct);                      /* Variable that holds the task struct */

        if (BluetoothTaskHandle == NULL || LinklayerTaskHandle == NULL)
        {
            err = SL_STATUS_FAIL;
        }
    }

    return err;
}

void BluetoothLLCallback()
{
    vRaiseEventFlagBasedOnContext(bluetooth_event_flags, BLUETOOTH_EVENT_FLAG_LL);
}

void BluetoothUpdate()
{
    vRaiseEventFlagBasedOnContext(bluetooth_event_flags, BLUETOOTH_EVENT_FLAG_STACK);
}
// Bluetooth task, it waits for events from bluetooth and handles them
void BluetoothTask(void * p)
{
    EventBits_t flags = BLUETOOTH_EVENT_FLAG_EVT_HANDLED | BLUETOOTH_EVENT_FLAG_STACK;
    TickType_t xTicksToWait;

    sli_bgapi_set_cmd_handler_delegate(sli_bt_cmd_handler_rtos_delegate);

    while (1)
    {
        // Command needs to be sent to Bluetooth stack
        if (flags & BLUETOOTH_EVENT_FLAG_CMD_WAITING)
        {
            uint32_t header              = command_header;
            sl_bgapi_handler cmd_handler = command_handler_func;
            sli_bgapi_cmd_handler_delegate(header, cmd_handler, (void *) command_data);
            command_handler_func = NULL;
            flags &= ~BLUETOOTH_EVENT_FLAG_CMD_WAITING;
            vRaiseEventFlagBasedOnContext(bluetooth_event_flags, BLUETOOTH_EVENT_FLAG_RSP_WAITING);
        }

        // Run Bluetooth stack. Pop the next event for application
        sl_bt_run();
        // Bluetooth stack needs updating, and evt can be used
        if (sl_bt_event_pending() && (flags & BLUETOOTH_EVENT_FLAG_EVT_HANDLED))
        { // update bluetooth & read event
            sl_status_t status = sl_bt_pop_event((sl_bt_msg_t *) bluetooth_evt);
            if (SL_STATUS_OK != status)
            {
                continue;
            }
            flags &= ~BLUETOOTH_EVENT_FLAG_EVT_HANDLED;
            vRaiseEventFlagBasedOnContext(bluetooth_event_flags, BLUETOOTH_EVENT_FLAG_EVT_WAITING);
        }

        xTicksToWait = sli_bt_can_sleep_ticks();
        if (xTicksToWait == 0 && (flags & BLUETOOTH_EVENT_FLAG_EVT_HANDLED))
        {
            flags |= BLUETOOTH_EVENT_FLAG_STACK;
            continue;
        }
        flags |= xEventGroupWaitBits(bluetooth_event_flags, /* The event group being tested. */
                                     (BLUETOOTH_EVENT_FLAG_STACK + BLUETOOTH_EVENT_FLAG_EVT_HANDLED +
                                      BLUETOOTH_EVENT_FLAG_CMD_WAITING), /* The bits within the event group to wait for. */
                                     pdTRUE,         /* BLUETOOTH_EVENT_FLAG_LL should be cleared before returning. */
                                     pdFALSE,        /* Wait for all the bits to be set, not needed for single bit. */
                                     portMAX_DELAY); /* Wait for maximum duration for bit to be set. With 1 ms tick,
                                                       portMAX_DELAY will result in wait of 50 days*/
        if (((flags & BLUETOOTH_EVENT_FLAG_STACK) == 0) && ((flags & BLUETOOTH_EVENT_FLAG_EVT_HANDLED) == 0) &&
            ((flags & BLUETOOTH_EVENT_FLAG_CMD_WAITING) == 0))
        {
            // timeout occurred, set the flag to update the Bluetooth stack
            flags |= BLUETOOTH_EVENT_FLAG_STACK;
        }
    }
}

static void LinklayerTask(void * p_arg)
{
    (void) p_arg;

    while (1)
    {
        EventBits_t uxBits;

        uxBits = xEventGroupWaitBits(bluetooth_event_flags,   /* The event group being tested. */
                                     BLUETOOTH_EVENT_FLAG_LL, /* The bits within the event group to wait for. */
                                     pdTRUE,                  /* BLUETOOTH_EVENT_FLAG_LL should be cleared before returning. */
                                     pdTRUE,                  /* Wait for all the bits to be set, not needed for single bit. */
                                     portMAX_DELAY);          /* Wait for maximum duration for bit to be set. With 1 ms tick,
                                                                 portMAX_DELAY will result in wait of 50 days*/

        if (uxBits & BLUETOOTH_EVENT_FLAG_LL)
        {
            sl_bt_priority_handle();
        }
    }
}

// hooks for API
// called from tasks using BGAPI
void sli_bt_cmd_handler_rtos_delegate(uint32_t header, sl_bgapi_handler handler, const void * payload)
{
    EventBits_t uxBits;

    command_header       = header;
    command_handler_func = handler;
    command_data         = (void *) payload;
    // Command structure is filled, notify the stack
    vRaiseEventFlagBasedOnContext(bluetooth_event_flags, BLUETOOTH_EVENT_FLAG_CMD_WAITING);

    // wait for response
    uxBits = xEventGroupWaitBits(bluetooth_event_flags,            /* The event group being tested. */
                                 BLUETOOTH_EVENT_FLAG_RSP_WAITING, /* The bits within the event group to wait for. */
                                 pdTRUE,                           /* BLUETOOTH_EVENT_FLAG_LL should be cleared before returning. */
                                 pdTRUE,                           /* Wait for all the bits to be set, not needed for single bit. */
                                 portMAX_DELAY);                   /* Wait for maximum duration for bit to be set. With 1 ms tick,
                                                                      portMAX_DELAY will result in wait of 50 days*/
    (void) uxBits;
}

void BluetoothPend(void)
{
    xSemaphoreTake(BluetoothMutex, portMAX_DELAY);
}
void BluetoothPost(void)
{
    xSemaphoreGive(BluetoothMutex);
}

void vApplicationMallocFailedHook(void)
{
    /* Called if a call to pvPortMalloc() fails because there is insufficient
    free memory available in the FreeRTOS heap.  pvPortMalloc() is called
    internally by FreeRTOS API functions that create tasks, queues, software
    timers, and semaphores.  The size of the FreeRTOS heap is set by the
    configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */

    EFR32_LOG("Failed do a malloc on HEAP. Is it too small ?");

    /* Force an assert. */
    configASSERT((volatile void *) NULL);
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char * pcTaskName)
{
    //(void) pcTaskName;
    (void) pxTask;

    /* Run time stack overflow checking is performed if
    configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected. */
    EFR32_LOG("TASK OVERFLOW");
    EFR32_LOG(pcTaskName);
    /* Force an assert. */
    configASSERT((volatile void *) NULL);
}

void vApplicationTickHook(void) {}

/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
void vApplicationGetIdleTaskMemory(StaticTask_t ** ppxIdleTaskTCBBuffer, StackType_t ** ppxIdleTaskStackBuffer,
                                   uint32_t * pulIdleTaskStackSize)
{
    /* If the buffers to be provided to the Idle task are declared inside this
    function then they must be declared static - otherwise they will be allocated on
    the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory(StaticTask_t ** ppxTimerTaskTCBBuffer, StackType_t ** ppxTimerTaskStackBuffer,
                                    uint32_t * pulTimerTaskStackSize)
{
    /* If the buffers to be provided to the Timer task are declared inside this
    function then they must be declared static - otherwise they will be allocated on
    the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
    task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

void vRaiseEventFlagBasedOnContext(EventGroupHandle_t xEventGroup, EventBits_t uxBitsToWaitFor)
{
    EventBits_t eventBits;
    BaseType_t eventBitsFromISRStatus;
    BaseType_t higherPrioTaskWoken = pdFALSE;

    if (xPortIsInsideInterrupt())
    {
        eventBitsFromISRStatus = xEventGroupSetBitsFromISR(xEventGroup, uxBitsToWaitFor, &higherPrioTaskWoken);
        if (eventBitsFromISRStatus != pdFAIL)
        {
#ifdef portYIELD_FROM_ISR
            portYIELD_FROM_ISR(higherPrioTaskWoken);
#elif portEND_SWITCHING_ISR // portYIELD_FROM_ISR or portEND_SWITCHING_ISR
            portEND_SWITCHING_ISR(higherPrioTaskWoken);
#else                       // portYIELD_FROM_ISR or portEND_SWITCHING_ISR
#error "Must have portYIELD_FROM_ISR or portEND_SWITCHING_ISR"
#endif // portYIELD_FROM_ISR or portEND_SWITCHING_ISR
        }
    }
    else
    {
        eventBits = xEventGroupSetBits(xEventGroup, uxBitsToWaitFor);
        (void) eventBits;
    }
}

BaseType_t vSendToQueueBasedOnContext(QueueHandle_t xQueue, void * xItemToQueue, TickType_t xTicksToWait,
                                      BaseType_t * pxHigherPriorityTaskWoken)
{
    BaseType_t status;
    BaseType_t higherPrioTaskWoken = pdFALSE;

    if (xPortIsInsideInterrupt())
    {
        status = xQueueSendFromISR(xQueue, xItemToQueue, &higherPrioTaskWoken);
    }
    else
    {
        status = xQueueSend(xQueue, xItemToQueue, xTicksToWait);
    }

    if (pxHigherPriorityTaskWoken != NULL)
    {
        *pxHigherPriorityTaskWoken = higherPrioTaskWoken;
    }

    return status;
}
