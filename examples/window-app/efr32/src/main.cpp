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

#include <AppConfig.h>
#include <WindowApp.h>

#include "init_efrPlatform.h"
#include "sl_simple_button_instances.h"
#include "sl_system_kernel.h"
#include <DeviceInfoProviderImpl.h>
#include <app/server/Server.h>
#include <matter_config.h>

#define BLE_DEV_NAME "Silabs-Window"
using namespace ::chip::DeviceLayer;

#define UNUSED_PARAMETER(a) (a = a)

volatile int apperror_cnt;
static chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;

// ================================================================================
// Main Code
// ================================================================================
int main(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    init_efrPlatform();
    if (EFR32MatterConfig::InitMatter(BLE_DEV_NAME) != CHIP_NO_ERROR)
        appError(CHIP_ERROR_INTERNAL);

    gExampleDeviceInfoProvider.SetStorageDelegate(&chip::Server::GetInstance().GetPersistentStorage());
    chip::DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);

    WindowApp & app = WindowApp::Instance();

    EFR32_LOG("Starting App");
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    err = app.Init();
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    if (err != CHIP_NO_ERROR)
    {
        EFR32_LOG("App Init failed");
        appError(err);
    }

    err = app.Start();
    if (err != CHIP_NO_ERROR)
    {
        EFR32_LOG("App Start failed");
        appError(err);
    }

    app.Finish();
    return err.AsInteger();
}
