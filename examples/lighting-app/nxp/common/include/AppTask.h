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

#pragma once

#include "AppConfig.h"
#include "AppTaskFreeRTOS.h"

#include <platform/CHIPDeviceLayer.h>

namespace LightingApp {

class AppTask : public chip::NXP::App::AppTaskFreeRTOS
{
public:
    // AppTaskBase virtual methods
    bool CheckStateClusterHandler(void) override;
    CHIP_ERROR ProcessSetStateClusterHandler(void) override;

    // AppTaskFreeRTOS virtual methods
    void PreInitMatterStack() override;

    // This returns an instance of this class.
    static AppTask & GetDefaultInstance();
};

} // namespace LightingApp

/**
 * Returns the application-specific implementation of the AppTaskBase object.
 *
 * Applications can use this to gain access to features of the AppTaskBase
 * that are specific to the selected application.
 */
chip::NXP::App::AppTaskBase & GetAppTask();
