/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <FreeRTOS.h>
#include <MemMonitoring.h>
#include <plat.h>
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

    while (true)
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
#ifdef CFG_USE_PSRAM
        ChipLogProgress(NotSpecified, "PSRAM Heap, min left: %d, current left %d", xPortGetMinimumEverFreeHeapSizePsram(),
                        xPortGetFreeHeapSizePsram());
#endif
        ChipLogProgress(NotSpecified, "============================= / total run time %ld", pulTotalRunTime);

        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
