/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2021-2023 Google LLC.
 *    Copyright 2026 NXP
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
#include <EnergyEvseMain.h>

extern bool CHARGING_ON;

extern "C"{
void StartEVSESession();
}

#if configAPPLICATION_ALLOCATED_HEAP
uint8_t __attribute__((section(".heap"))) ucHeap[configTOTAL_HEAP_SIZE];
#endif

using namespace ::chip::DeviceLayer;
using namespace chip::app::Clusters::DeviceEnergyManagement;
using namespace chip::app::Clusters::DeviceEnergyManagement::Attributes;

int main(int argc, char * argv[])
{
    TaskHandle_t taskHandle;

    PlatformMgrImpl().HardwareInit();
    TEMPORARY_RETURN_IGNORED chip::NXP::App::GetAppTask().Start();

    StartEVSESession();

    vTaskStartScheduler();
}

#if (defined(configCHECK_FOR_STACK_OVERFLOW) && (configCHECK_FOR_STACK_OVERFLOW > 0))
void vApplicationStackOverflowHook(TaskHandle_t xTask, char * pcTaskName)
{
    assert(0);
}
#endif

#define EVSE_TASK_SIZE (((unsigned short)90)+800)
TaskHandle_t EVSETaskHandle;
bool EVSE_task_running = false;


void EVSE_task(void *pvParameters)
{
    for (;;){
    if((CHARGING_ON) == true && (EVSEApp::AppTask::GetDefaultInstance().isCommissioningcomplete()))
    {
        EVSEApp::AppTask::GetDefaultInstance().CharingUpdateHandler();
    }
	else{
		
	}
    vTaskDelay(OSA_MsecToTicks(5000));
    }
}

void StartEVSESession()
{
    if (EVSE_task_running) {
        return;
    }

    if (xTaskCreate(&EVSE_task, "EVSE_task", EVSE_TASK_SIZE, NULL, 1, &EVSETaskHandle) != pdPASS) {
        ChipLogError(DeviceLayer, "Failed to start EVSE task");
        assert(false);
    }
    ChipLogProgress(DeviceLayer, "EVSE task started.");
}