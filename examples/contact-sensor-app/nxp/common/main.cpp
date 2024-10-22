/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "AppTask.h"
#include "FreeRTOS.h"

#if configAPPLICATION_ALLOCATED_HEAP
uint8_t __attribute__((section(".heap"))) ucHeap[configTOTAL_HEAP_SIZE];
#endif

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
