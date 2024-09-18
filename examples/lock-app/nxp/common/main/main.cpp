/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2021-2023 Google LLC.
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

// ================================================================================
// Main Code
// ================================================================================

#include "FreeRTOS.h"
#include <AppTask.h>
#include <lib/core/CHIPError.h>
#include <lib/support/logging/CHIPLogging.h>

#if configAPPLICATION_ALLOCATED_HEAP
uint8_t __attribute__((section(".heap"))) ucHeap[configTOTAL_HEAP_SIZE];
#endif

using namespace ::chip::DeviceLayer;

#if FSL_OSA_MAIN_FUNC_ENABLE
extern "C" void main_task(void const * argument)
{
    chip::DeviceLayer::PlatformMgrImpl().HardwareInit();
    chip::NXP::App::GetAppTask().Start();
}
#else
extern "C" int main(int argc, char * argv[])
{
    chip::DeviceLayer::PlatformMgrImpl().HardwareInit();
    chip::NXP::App::GetAppTask().Start();
    vTaskStartScheduler();
}
#endif

#if (defined(configCHECK_FOR_STACK_OVERFLOW) && (configCHECK_FOR_STACK_OVERFLOW > 0))
void vApplicationStackOverflowHook(TaskHandle_t xTask, char * pcTaskName)
{
    assert(0);
}
#endif
