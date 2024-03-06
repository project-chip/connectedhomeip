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

#include <AppTask.h>

#include "AppConfig.h"

#include <DeviceInfoProviderImpl.h>
#include <MatterConfig.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <examples/platform/silabs/SilabsDeviceAttestationCreds.h>

#include <platform/silabs/platformAbstraction/SilabsPlatform.h>

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "event_groups.h"
#include "task.h"

/**********************************************************
 * Defines
 *********************************************************/

#define MAIN_TASK_STACK_SIZE (1024 * 5)
#define MAIN_TASK_PRIORITY (configMAX_PRIORITIES - 1)

using namespace ::chip;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Credentials;
using namespace chip::DeviceLayer::Silabs;

TaskHandle_t main_Task;
void application_start(void * unused);
volatile int apperror_cnt;
static chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;

// ================================================================================
// Main Code
// ================================================================================
int main(void)
{
    GetPlatform().Init();

    xTaskCreate(application_start, "main_task", MAIN_TASK_STACK_SIZE, NULL, MAIN_TASK_PRIORITY, &main_Task);

    SILABS_LOG("Starting scheduler");
    GetPlatform().StartScheduler();

    // Should never get here.
    chip::Platform::MemoryShutdown();
    SILABS_LOG("vTaskStartScheduler() failed");
    appError(CHIP_ERROR_INTERNAL);
}

void application_start(void * unused)
{
    CHIP_ERROR err = SilabsMatterConfig::InitMatter(BLE_DEV_NAME);
    if (err != CHIP_NO_ERROR)
        appError(err);

    gExampleDeviceInfoProvider.SetStorageDelegate(&chip::Server::GetInstance().GetPersistentStorage());
    chip::DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Credentials::Silabs::GetSilabsDacProvider());
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    SILABS_LOG("Starting App Task");
    err = AppTask::GetAppTask().StartAppTask();
    if (err != CHIP_NO_ERROR)
        appError(err);

    vTaskDelete(main_Task);
}
