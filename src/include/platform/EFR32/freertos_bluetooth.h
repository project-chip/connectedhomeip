/*
 *
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

#ifndef FREERTOS_BLUETOOTH_H
#define FREERTOS_BLUETOOTH_H

#if __cplusplus
extern "C" {
#endif

#include "rtos_gecko.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "event_groups.h"

// Bluetooth event flag group
extern EventGroupHandle_t bluetooth_event_flags;
// Bluetooth event flag definitions
#define BLUETOOTH_EVENT_FLAG_STACK (0x01)       // Bluetooth task needs an update
#define BLUETOOTH_EVENT_FLAG_LL (0x02)          // Linklayer task needs an update
#define BLUETOOTH_EVENT_FLAG_CMD_WAITING (0x04) // BGAPI command is waiting to be processed
#define BLUETOOTH_EVENT_FLAG_RSP_WAITING (0x08) // BGAPI response is waiting to be processed
#define BLUETOOTH_EVENT_FLAG_EVT_WAITING (0x10) // BGAPI event is waiting to be processed
#define BLUETOOTH_EVENT_FLAG_EVT_HANDLED (0x20) // BGAPI event is handled

// Bluetooth event data pointer
extern volatile struct gecko_cmd_packet *bluetooth_evt;

// Function prototype for initializing Bluetooth stack.
typedef errorcode_t (*bluetooth_stack_init_func)();

/**
 * Start Bluetooth tasks. The given Bluetooth stack initialization function
 * will be called at a proper time. Application should not initialize
 * Bluetooth stack anywhere else.
 *
 * @param ll_priority link layer task priority
 * @param stack_priority Bluetooth stack task priority
 * @param initialize_bluetooth_stack The function for initializing Bluetooth stack
 */
errorcode_t bluetooth_start(UBaseType_t               ll_priority,
                            UBaseType_t               stack_priority,
                            bluetooth_stack_init_func initialize_bluetooth_stack);

// Set the callback for wakeup, Bluetooth task will call this when it has a new event
// It must only used to wake up application task, for example by posting task semaphore
typedef void (*wakeupCallback)(void);
void BluetoothSetWakeupCallback(wakeupCallback cb);
// Bluetooth stack needs an update
extern void BluetoothUpdate(void);
// Linklayer is updated
extern void BluetoothLLCallback(void);

// Mutex functions for using Bluetooth from multiple tasks
void BluetoothPend(void);
void BluetoothPost(void);

EventBits_t vRaiseEventFlagBasedOnContext(EventGroupHandle_t xEventGroup,
                                          EventBits_t        uxBitsToWaitFor,
                                          BaseType_t *       pxHigherPriorityTaskWoken);
EventBits_t vSendToQueueBasedOnContext(QueueHandle_t xQueue,
                                       void *        xItemToQueue,
                                       TickType_t    xTicksToWait,
                                       BaseType_t *  pxHigherPriorityTaskWoken);

#if __cplusplus
}
#endif

#endif // FREERTOS_BLUETOOTH_H
