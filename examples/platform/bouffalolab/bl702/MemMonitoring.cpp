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
#include "FreeRTOS.h"
#include <platform/CHIPDeviceLayer.h>

static StackType_t monitoringStack[MONITORING_STACK_SIZE_byte / sizeof(StackType_t)];
static StaticTask_t monitoringTaskStruct;

void MemMonitoring::startHeapMonitoring()
{
    xTaskCreateStatic(HeapMonitoring, "Monitoring", MONITORING_STACK_SIZE_byte / sizeof(StackType_t), NULL, 1, monitoringStack,
                      &monitoringTaskStruct);
}

void MemMonitoring::HeapMonitoring(void * pvParameter)
{
    TaskStatus_t * pTaskStatus = NULL;
    char taskState[]           = { 'X', 'R', 'B', 'S', 'D' }; // eRunning, eReady, eBlocked, eSuspended, eDeleted
    uint32_t pulTotalRunTime;

    while (1)
    {
        ChipLogProgress(NotSpecified, "=============================");
        pTaskStatus = (TaskStatus_t *) malloc(uxTaskGetNumberOfTasks() * sizeof(TaskStatus_t));
        if (pTaskStatus)
        {
            ChipLogProgress(NotSpecified, "Task Name\tstate\tpriority\thighWaterMark");
            uxTaskGetSystemState(pTaskStatus, uxTaskGetNumberOfTasks(), &pulTotalRunTime);
            for (uint32_t i = 0; i < uxTaskGetNumberOfTasks(); i++)
            {
                ChipLogProgress(NotSpecified, "%s%s\t%c\t%ld/%ld\t\t%d", (pTaskStatus + i)->pcTaskName,
                                strlen((pTaskStatus + i)->pcTaskName) < 7 ? "\t" : "",
                                (pTaskStatus + i)->eCurrentState > eDeleted ? 'N' : taskState[(pTaskStatus + i)->eCurrentState],
                                (pTaskStatus + i)->uxCurrentPriority, (pTaskStatus + i)->uxBasePriority,
                                (pTaskStatus + i)->usStackHighWaterMark);
            }
            free(pTaskStatus);
        }
        ChipLogProgress(NotSpecified, "\r\n");
        ChipLogProgress(NotSpecified, "SRAM Heap, min left: %d, current left %d", xPortGetMinimumEverFreeHeapSize(),
                        xPortGetFreeHeapSize());
        ChipLogProgress(NotSpecified, "PSRAM Heap, min left: %d, current left %d", xPortGetMinimumEverFreeHeapSizePsram(),
                        xPortGetFreeHeapSizePsram());
        ChipLogProgress(NotSpecified, "============================= / total run time %ld", pulTotalRunTime);

        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
