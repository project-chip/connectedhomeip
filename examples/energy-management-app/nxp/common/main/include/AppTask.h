/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2021-2023 Google LLC.
 *    Copyright 2024,2026 NXP
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

#if CONFIG_APP_FREERTOS_OS
#include "AppTaskFreeRTOS.h"
#else
#include "AppTaskZephyr.h"
#endif

namespace EVSEApp {
#if CONFIG_APP_FREERTOS_OS
class AppTask : public chip::NXP::App::AppTaskFreeRTOS
#else
class AppTask : public chip::NXP::App::AppTaskZephyr
#endif
{
public:
    ~AppTask() override{};
    void PostInitMatterStack(void) override;
    void PreInitMatterStack(void) override;
    void AppMatter_RegisterCustomCliCommands(void) override;
    void PostInitMatterServerInstance(void) override;
    // This returns an instance of this class.
    static AppTask & GetDefaultInstance();

    bool CheckStateClusterHandler(void) override;
    CHIP_ERROR ProcessSetStateClusterHandler(void) override;
    bool isCommissioningcomplete(void);
    void CharingUpdateHandler(void);
    void StopChargingHandler(void);

private:
    static AppTask sAppTask;

    static void UpdateChargingInternal(intptr_t arg);
    static void StopChargingInternal(intptr_t arg);
};
} // namespace EVSEApp

/**
 * Returns the application-specific implementation of the AppTaskBase object.
 *
 * Applications can use this to gain access to features of the AppTaskBase
 * that are specific to the selected application.
 */
chip::NXP::App::AppTaskBase & GetAppTask();
