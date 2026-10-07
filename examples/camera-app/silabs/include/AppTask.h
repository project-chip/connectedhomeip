/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <credentials/GroupDataProvider.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <platform/CHIPDeviceLayer.h>

#include "BaseApplication.h"

class AppTask : public BaseApplication
{
public:
    AppTask() = default;

    static AppTask & GetAppTask() { return sAppTask; }

    static void AppTaskMain(void * pvParameter);

    CHIP_ERROR StartAppTask();

    static void ButtonEventHandler(uint8_t button, uint8_t btnAction);

    // Code-driven data model initialization hooks required by MatterConfig.cpp
    static CHIP_ERROR InitCodeDrivenDataModel(chip::PersistentStorageDelegate & storage,
                                              chip::Credentials::GroupDataProvider * groupDataProvider,
                                              chip::Crypto::SessionKeystore * sessionKeyStore);
    static chip::app::CodeDrivenDataModelProvider * GetDataModelProvider();

private:
    static AppTask sAppTask;

    CHIP_ERROR AppInit() override;
};
