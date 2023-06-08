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

#pragma once

#include "AppEvent.h"
#include "LEDWidget.h"
#include "PumpManager.h"

#include <platform/CHIPDeviceLayer.h>

#if CONFIG_CHIP_FACTORY_DATA
#include <platform/nrfconnect/FactoryDataProvider.h>
#else
#include <platform/nrfconnect/DeviceInstanceInfoProviderImpl.h>
#endif

#ifdef CONFIG_MCUMGR_TRANSPORT_BT
#include "DFUOverSMP.h"
#endif

struct k_timer;

class AppTask
{
public:
    static AppTask & Instance(void)
    {
        static AppTask sAppTask;
        return sAppTask;
    };
    CHIP_ERROR StartApp();

    static void PostStartActionRequest(int32_t actor, PumpManager::Action_t action);
    static void UpdateClusterState();
    static void PostEvent(const AppEvent & event);

private:
    CHIP_ERROR Init();

    static void CancelTimer();
    static void StartTimer(uint32_t timeoutInMs);

    static void ActionInitiated(PumpManager::Action_t action, int32_t actor);
    static void ActionCompleted(PumpManager::Action_t action, int32_t actor);

    static void DispatchEvent(const AppEvent & event);
    static void FunctionTimerEventHandler(const AppEvent & event);
    static void FunctionHandler(const AppEvent & event);
    static void StartBLEAdvertisementHandler(const AppEvent & event);
    static void UpdateLedStateEventHandler(const AppEvent & event);
    static void StartActionEventHandler(const AppEvent & event);

    static void ChipEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);
    static void ButtonEventHandler(uint32_t buttonState, uint32_t hasChanged);
    static void LEDStateUpdateHandler(LEDWidget & ledWidget);
    static void FunctionTimerTimeoutCallback(k_timer * timer);
    static void UpdateStatusLED();

    FunctionEvent mFunction   = FunctionEvent::NoneSelected;
    bool mFunctionTimerActive = false;

#if CONFIG_CHIP_FACTORY_DATA
    chip::DeviceLayer::FactoryDataProvider<chip::DeviceLayer::InternalFlashFactoryData> mFactoryDataProvider;
#endif
};
