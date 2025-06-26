/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#ifndef APP_TASK_H
#define APP_TASK_H

#include <stdbool.h>
#include <stdint.h>

#include "AppEvent.h"
#include "ThermostaticRadiatorValveManager.h"
#include <BaseAppTask.h>

#include "FreeRTOS.h"
#include "timers.h" // provides FreeRTOS timer support
#include <platform/CHIPDeviceLayer.h>

#include <platform/qpg/FactoryDataProvider.h>

#ifdef APP_NAME
#undef APP_NAME
#endif
#define APP_NAME "thermostaticRadiatorValve-app"

class AppTask : public BaseAppTask
{

public:
    // Base class overrides
    CHIP_ERROR Init() override;
    bool ButtonEventHandler(uint8_t btnIdx, bool btnPressed) override;
    void PowerCycleExpiredHandler(uint8_t resetCounts) override;

    static AppTask & GetAppTask(void) { return sAppTask; }

    // Static wrapper that redirects to the instance method
    static void InitServerWrapper(intptr_t arg) { sAppTask.InitServer(arg); }
    static void ButtonEventHandlerWrapper(uint8_t button, bool pressed) { sAppTask.ButtonEventHandler(button, pressed); }
    static void PowerCycleExpiredHandlerWrapper(uint8_t resetCounts) { sAppTask.PowerCycleExpiredHandler(resetCounts); }

private:
    static AppTask sAppTask;
};

#endif // APP_TASK_H
