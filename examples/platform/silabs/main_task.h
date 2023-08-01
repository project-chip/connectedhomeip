/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

/**********************************************************
 * Includes
 *********************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"
int32_t wfx_wifi_rsi_init(void);

#ifdef __cplusplus
}
#endif

/**********************************************************
 * Defines
 *********************************************************/
#define MAIN_TASK_STACK_SIZE (1024 * 8)
#define MAIN_TASK_PRIORITY 56

const osThreadAttr_t thread_attributes = {
  .name       = "app",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = MAIN_TASK_STACK_SIZE,
  .priority   = (osPriority_t)MAIN_TASK_PRIORITY,
  .tz_module  = 0,
  .reserved   = 0,
};

osThreadId_t main_Task;

void application_start(const void *unused);
