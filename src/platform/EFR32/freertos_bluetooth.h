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

#pragma once

#if __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "event_groups.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

#include "sl_bt_api.h"

#define BLE_STACK_TASK_NAME "BLE_STACK"
#define BLE_LINK_TASK_NAME "BLE_LINK"

// Bluetooth event flag group
extern EventGroupHandle_t bluetooth_event_flags;
// Bluetooth event flag definitions
#define BLUETOOTH_EVENT_FLAG_STACK (0x01u)       // Bluetooth task needs an update
#define BLUETOOTH_EVENT_FLAG_LL (0x02u)          // Linklayer task needs an update
#define BLUETOOTH_EVENT_FLAG_CMD_WAITING (0x04u) // BGAPI command is waiting to be processed
#define BLUETOOTH_EVENT_FLAG_RSP_WAITING (0x08u) // BGAPI response is waiting to be processed
#define BLUETOOTH_EVENT_FLAG_EVT_WAITING (0x10u) // BGAPI event is waiting to be processed
#define BLUETOOTH_EVENT_FLAG_EVT_HANDLED (0x20u) // BGAPI event is handled

// Bluetooth event data pointer
extern volatile sl_bt_msg_t * bluetooth_evt;

// Function prototype for initializing Bluetooth stack.
typedef sl_status_t (*bluetooth_stack_init_func)();

/**
 * Start Bluetooth tasks. The given Bluetooth stack initialization function
 * will be called at a proper time. Application should not initialize
 * Bluetooth stack anywhere else.
 *
 * @param ll_priority link layer task priority
 * @param stack_priority Bluetooth stack task priority
 * @param initialize_bluetooth_stack The function for initializing Bluetooth stack
 */
sl_status_t bluetooth_start(UBaseType_t ll_priority, UBaseType_t stack_priority,
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

void vRaiseEventFlagBasedOnContext(EventGroupHandle_t xEventGroup, EventBits_t uxBitsToWaitFor);
BaseType_t vSendToQueueBasedOnContext(QueueHandle_t xQueue, void * xItemToQueue, TickType_t xTicksToWait,
                                      BaseType_t * pxHigherPriorityTaskWoken);

#if __cplusplus
}
#endif
