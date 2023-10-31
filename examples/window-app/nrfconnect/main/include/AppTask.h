/*
 *    Copyright (c) 2022 Project CHIP Authors
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
#include "WindowCovering.h"
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
struct Identify;

class AppTask
{
public:
    static AppTask & Instance(void)
    {
        static AppTask sAppTask;
        return sAppTask;
    };
    CHIP_ERROR StartApp();

    static void IdentifyStartHandler(Identify *);
    static void IdentifyStopHandler(Identify *);

private:
    CHIP_ERROR Init();
    void ToggleMoveType();

    static void CancelTimer();
    static void StartTimer(uint32_t timeoutMs);

    static void PostEvent(const AppEvent & event);
    static void DispatchEvent(const AppEvent & event);
    static void FunctionTimerEventHandler(const AppEvent & event);
    static void FunctionHandler(const AppEvent & event);
    static void UpdateLedStateEventHandler(const AppEvent & event);
    static void StartBLEAdvertisementHandler(const AppEvent & event);
    static void MovementTimerEventHandler(const AppEvent & event);
    static void OpenHandler(const AppEvent & event);
    static void CloseHandler(const AppEvent & event);

    static void TimerTimeoutCallback(k_timer * timer);
    static void FunctionTimerTimeoutCallback(k_timer * timer);
    static void LEDStateUpdateHandler(LEDWidget & ledWidget);

    static void ChipEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);
    static void ButtonEventHandler(uint32_t buttonState, uint32_t hasChanged);
    static void UpdateStatusLED();

    FunctionEvent mFunction{ FunctionEvent::NoneSelected };
    OperationalState mMoveType{ OperationalState::MovingUpOrOpen };
    bool mFunctionTimerActive{ false };
    bool mMovementTimerActive{ false };
    bool mOpenButtonIsPressed{ false };
    bool mCloseButtonIsPressed{ false };
    bool mMoveTypeRecentlyChanged{ false };

#if CONFIG_CHIP_FACTORY_DATA
    chip::DeviceLayer::FactoryDataProvider<chip::DeviceLayer::InternalFlashFactoryData> mFactoryDataProvider;
#endif
};
