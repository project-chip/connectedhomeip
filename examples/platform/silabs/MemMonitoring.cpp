/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "MemMonitoring.h"

#include "AppConfig.h"
#include <cmsis_os2.h>
#include <platform/CHIPDeviceLayer.h>
#include <sl_cmsis_os2_common.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {

static osThreadId_t sMonitorThreadHandle;
constexpr uint32_t kMonitorTaskSize = 1024;
static uint8_t monitorStack[kMonitorTaskSize];
static osThread_t sMonitorTaskControlBlock;
constexpr osThreadAttr_t kMonitorTaskAttr = { .name       = "MemMonitor",
                                              .attr_bits  = osThreadDetached,
                                              .cb_mem     = &sMonitorTaskControlBlock,
                                              .cb_size    = osThreadCbSize,
                                              .stack_mem  = monitorStack,
                                              .stack_size = kMonitorTaskSize,
                                              .priority   = osPriorityLow };

size_t nbAllocSuccess        = 0;
size_t nbFreeSuccess         = 0;
size_t largestBlockAllocated = 0;

void MemMonitoring::StartMonitor()
{
    sMonitorThreadHandle = osThreadNew(MonitorTask, nullptr, &kMonitorTaskAttr);
}

void MemMonitoring::MonitorTask(void * pvParameter)
{
    uint32_t threadCount = osThreadGetCount();

    osThreadId_t * threadIdTable = new osThreadId_t[threadCount];
    //  Forms a table of the active thread ids
    osThreadEnumerate(threadIdTable, threadCount);

    while (true)
    {

        SILABS_LOG("=============================");
        SILABS_LOG(" ");
        SILABS_LOG("Largest Block allocated     %lu B", largestBlockAllocated);
        SILABS_LOG("Number Of Successful Alloc  %lu", nbAllocSuccess);
        SILABS_LOG("Number Of Successful Frees  %lu", nbFreeSuccess);
        SILABS_LOG(" ");

        SILABS_LOG("Thread stack highwatermark ");
        for (uint8_t tIdIndex = 0; tIdIndex < threadCount; tIdIndex++)
        {
            osThreadId_t tId = threadIdTable[tIdIndex];
            if (tId != sMonitorThreadHandle) // don't print stats for this current debug thread.
            {
                // The smallest amount of free stack space there has been since the thread creation
                SILABS_LOG("\t%-10s : %6lu B", osThreadGetName(tId), osThreadGetStackSpace(tId));
            }
        }

        SILABS_LOG(" ");
        SILABS_LOG("=============================");
        // run loop every 5 seconds
        osDelay(osKernelGetTickFreq() * 5);
    }

    // will never get here. Still, free allocated memory before exiting
    delete threadIdTable;
}

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip

extern "C" void memMonitoringTrackAlloc(void * ptr, size_t size)
{
    if (ptr != NULL)
    {
        chip::DeviceLayer::Silabs::nbAllocSuccess++;
        if (chip::DeviceLayer::Silabs::largestBlockAllocated < size)
        {
            chip::DeviceLayer::Silabs::largestBlockAllocated = size;
        }
    }
}

extern "C" void memMonitoringTrackFree(void * ptr, size_t size)
{
    chip::DeviceLayer::Silabs::nbFreeSuccess++;
}
