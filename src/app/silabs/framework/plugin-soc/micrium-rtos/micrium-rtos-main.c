/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
/***************************************************************************//**
 * @file
 * @brief Main entrypoint when using Micrium RTOS.
 *******************************************************************************
   ******************************************************************************/

#include PLATFORM_HEADER
#ifdef EMBER_AF_NCP
#include "app/framework/include/af-ncp.h"
#else // !EMBER_AF_NCP
#include "app/framework/include/af.h"
#endif // EMBER_AF_NCP
#include "sl_sleeptimer.h"

#include <kernel/include/os.h>
#include <kernel/include/os_trace.h>

#if defined(EMBER_AF_PLUGIN_BLE)
#include EMBER_AF_API_BLE
#endif // EMBER_AF_PLUGIN_BLE

#if defined(EMBER_AF_PLUGIN_MBEDTLS_MULTIPROTOCOL)
#include "mbedtls/threading.h"
#endif

#ifdef EMBER_AF_NCP
extern int emberAfMain(MAIN_FUNCTION_PARAMETERS);
extern boolean isZigbeeTaskPended;
#endif // EMBER_AF_NCP
//------------------------------------------------------------------------------
// Tasks variables and defines

#define BLE_LINK_LAYER_TASK_PRIORITY            4
#define BLE_STACK_TASK_PRIORITY                 5
#define ZIGBEE_STACK_TASK_PRIORITY              6

static OS_TCB zigbeeTaskControlBlock;
static CPU_STK zigbeeTaskStack[EMBER_AF_PLUGIN_MICRIUM_RTOS_ZIGBEE_STACK_SIZE];

#if defined(EMBER_AF_PLUGIN_MICRIUM_RTOS_APP_TASK1)
static OS_TCB applicationTask1ControlBlock;
static CPU_STK applicationTask1Stack[EMBER_AF_PLUGIN_MICRIUM_RTOS_APP_TASK1_STACK_SIZE];
#endif // EMBER_AF_PLUGIN_MICRIUM_RTOS_APP_TASK1

#if defined(EMBER_AF_PLUGIN_MICRIUM_RTOS_APP_TASK2)
static OS_TCB applicationTask2ControlBlock;
static CPU_STK applicationTask2Stack[EMBER_AF_PLUGIN_MICRIUM_RTOS_APP_TASK2_STACK_SIZE];
#endif // EMBER_AF_PLUGIN_MICRIUM_RTOS_APP_TASK2

#if defined(EMBER_AF_PLUGIN_MICRIUM_RTOS_APP_TASK3)
static OS_TCB applicationTask3ControlBlock;
static CPU_STK applicationTask3Stack[EMBER_AF_PLUGIN_MICRIUM_RTOS_APP_TASK3_STACK_SIZE];
#endif // EMBER_AF_PLUGIN_MICRIUM_RTOS_APP_TASK3

//------------------------------------------------------------------------------
// Forward and external declarations

static void zigbeeTask(void *p_arg);
static void initMicriumCpu(void);

extern void App_OS_SetAllHooks(void);
//------------------------------------------------------------------------------
// Main

int main(MAIN_FUNCTION_PARAMETERS)
{
  RTOS_ERR err;

  CPU_Init();
  halInit();
  initMicriumCpu();
#ifndef EMBER_AF_NCP
  // Not exist on NCP.
  emberAfMainInit();
#endif
  // Initialize tokens in zigbee before we call emberAfMain because we need to
  // upgrade any potential SimEEv2 data to NVM3 before we call gecko_stack_init
  // (a bluetooth call). If we don't, the bluetooth call only initializes NVM3
  // and wipes any SimEEv2 data present (it does not handle upgrading). The
  // second call of halStackInitTokens in the zigbee stack will be a no-op
  if (EMBER_SUCCESS != halStackInitTokens()) {
    assert(false);
  }

  OS_TRACE_INIT();
  OSInit(&err);
  assert(RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE);

  App_OS_SetAllHooks();

  // Create ZigBee task.
  // Note, this is gonna create the rest of the tasks down the road.
  OSTaskCreate(&zigbeeTaskControlBlock,
               "Zigbee Stack",
               zigbeeTask,
               NULL,
               ZIGBEE_STACK_TASK_PRIORITY,
               &zigbeeTaskStack[0],
               EMBER_AF_PLUGIN_MICRIUM_RTOS_ZIGBEE_STACK_SIZE / 10,
               EMBER_AF_PLUGIN_MICRIUM_RTOS_ZIGBEE_STACK_SIZE,
               0, // Not receiving messages
               0, // Default time quanta
               NULL, // No TCB extensions
               OS_OPT_TASK_STK_CLR | OS_OPT_TASK_STK_CHK,
               &err);
  assert(RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE);

  // Start the OS
  OSStart(&err);
  assert(RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE);
}

//------------------------------------------------------------------------------
// Internal APIs

// This can be called from ISR.
void emAfPluginMicriumRtosWakeUpZigbeeStackTask(void)
{
#ifdef EMBER_AF_NCP
  boolean taskPended;
  ATOMIC(taskPended = isZigbeeTaskPended; )
  // Post the semaphore only if task is pended.
  if (taskPended) {
#endif // EMBER_AF_NCP
  RTOS_ERR err;

  OSTaskSemPost(&zigbeeTaskControlBlock,
                OS_OPT_POST_NONE,
                &err);
  assert(RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE);
#ifdef EMBER_AF_NCP
  // Clear the flag as semaphore is posted.
  ATOMIC(isZigbeeTaskPended = false; )
}
#endif // EMBER_AF_NCP
}

#if defined(EMBER_AF_PLUGIN_BLE)
static errorcode_t initializeBluetooth(void)
{
  errorcode_t bleError = gecko_init(emberAfPluginBleGetConfig());
  if (bleError == bg_err_success) {
    gecko_init_multiprotocol(NULL);
  }
  return bleError;
}
#endif // EMBER_AF_PLUGIN_BLE

//------------------------------------------------------------------------------
// Implemented callbacks

bool emberRtosIdleHandler(uint32_t *idleTimeMs)
{
  return emberAfPluginIdleSleepRtosCallback(idleTimeMs, false);
}

void emberRtosStackWakeupIsrHandler(void)
{
  emAfPluginMicriumRtosWakeUpZigbeeStackTask();
}

//------------------------------------------------------------------------------
// Static functions

static void zigbeeTask(void *p_arg)
{
  RTOS_ERR err;

#if defined(EMBER_AF_PLUGIN_MBEDTLS_MULTIPROTOCOL)
  THREADING_setup();
#endif

#if defined(EMBER_AF_PLUGIN_BLE)
  BluetoothSetWakeupCallback(emAfPluginMicriumRtosWakeUpZigbeeStackTask);
  bluetooth_start(BLE_LINK_LAYER_TASK_PRIORITY, BLE_STACK_TASK_PRIORITY, initializeBluetooth);
#endif // EMBER_AF_PLUGIN_BLE

#if defined(EMBER_AF_PLUGIN_MICRIUM_RTOS_APP_TASK1)
  emberAfPluginMicriumRtosAppTask1InitCallback();

  // Create Application Task 1.
  OSTaskCreate(&applicationTask1ControlBlock,
               "Application (1)",
               emberAfPluginMicriumRtosAppTask1MainLoopCallback,
               NULL,
               EMBER_AF_PLUGIN_MICRIUM_RTOS_APP_TASK1_PRIORITY,
               &applicationTask1Stack[0],
               EMBER_AF_PLUGIN_MICRIUM_RTOS_APP_TASK1_STACK_SIZE / 10,
               EMBER_AF_PLUGIN_MICRIUM_RTOS_APP_TASK1_STACK_SIZE,
               0, // Not receiving messages
               0, // Default time quanta
               NULL, // No TCB extensions
               OS_OPT_TASK_STK_CLR | OS_OPT_TASK_STK_CHK,
               &err);
  assert(RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE);

#endif // EMBER_AF_PLUGIN_MICRIUM_RTOS_APP_TASK1

#if defined(EMBER_AF_PLUGIN_MICRIUM_RTOS_APP_TASK2)

  emberAfPluginMicriumRtosAppTask2InitCallback();

  // Create Application Task 2.
  OSTaskCreate(&applicationTask2ControlBlock,
               "Application (2)",
               emberAfPluginMicriumRtosAppTask2MainLoopCallback,
               NULL,
               EMBER_AF_PLUGIN_MICRIUM_RTOS_APP_TASK2_PRIORITY,
               &applicationTask2Stack[0],
               EMBER_AF_PLUGIN_MICRIUM_RTOS_APP_TASK2_STACK_SIZE / 10,
               EMBER_AF_PLUGIN_MICRIUM_RTOS_APP_TASK2_STACK_SIZE,
               0, // Not receiving messages
               0, // Default time quanta
               NULL, // No TCB extensions
               OS_OPT_TASK_STK_CLR | OS_OPT_TASK_STK_CHK,
               &err);
  assert(RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE);

#endif // EMBER_AF_PLUGIN_MICRIUM_RTOS_APP_TASK2

#if defined(EMBER_AF_PLUGIN_MICRIUM_RTOS_APP_TASK3)

  emberAfPluginMicriumRtosAppTask3InitCallback();

  // Create Application Task 3.
  OSTaskCreate(&applicationTask3ControlBlock,
               "Application (3)",
               emberAfPluginMicriumRtosAppTask3MainLoopCallback,
               NULL,
               EMBER_AF_PLUGIN_MICRIUM_RTOS_APP_TASK3_PRIORITY,
               &applicationTask3Stack[0],
               EMBER_AF_PLUGIN_MICRIUM_RTOS_APP_TASK3_STACK_SIZE / 10,
               EMBER_AF_PLUGIN_MICRIUM_RTOS_APP_TASK3_STACK_SIZE,
               0, // Not receiving messages
               0, // Default time quanta
               NULL, // No TCB extensions
               OS_OPT_TASK_STK_CLR | OS_OPT_TASK_STK_CHK,
               &err);
  assert(RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE);

#endif // EMBER_AF_PLUGIN_MICRIUM_RTOS_APP_TASK3

  emberAfMain();
}

// Register interrupt vectors with the OS
static void initMicriumCpu(void)
{
  // Radio Interrupts can optionally be configured non-kernel aware at this point
}
