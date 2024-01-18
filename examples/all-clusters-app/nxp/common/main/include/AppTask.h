/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2021-2023 Google LLC.
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

#include <stdbool.h>
#include <stdint.h>

#include "AppEvent.h"
#include "DeviceCallbacks.h"

class AppTask
{
public:
    CHIP_ERROR StartAppTask();
    static void AppTaskMain(void * pvParameter);

    void PostEvent(const AppEvent * event);

    /* Commissioning handlers */
    void StartCommissioningHandler(void);
    void StopCommissioningHandler(void);
    void SwitchCommissioningStateHandler(void);

    /* FactoryResetHandler */
    void FactoryResetHandler(void);

private:
    DeviceCallbacks deviceCallbacks;

    friend AppTask & GetAppTask(void);

    CHIP_ERROR Init();
    void DispatchEvent(AppEvent * event);
    CHIP_ERROR DisplayDeviceInformation(void);

    /* Functions that would be called in the Matter task context */
    static void StartCommissioning(intptr_t arg);
    static void StopCommissioning(intptr_t arg);
    static void SwitchCommissioningState(intptr_t arg);
    static void InitServer(intptr_t arg);

    static AppTask sAppTask;
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}
