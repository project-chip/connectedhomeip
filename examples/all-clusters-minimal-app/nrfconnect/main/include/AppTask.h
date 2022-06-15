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

#include <platform/CHIPDeviceLayer.h>

#if CONFIG_CHIP_FACTORY_DATA
#include <platform/nrfconnect/FactoryDataProvider.h>
#endif

struct k_timer;
class AppEvent;
class LEDWidget;

class AppTask
{
public:
    static AppTask & Instance(void)
    {
        static AppTask sAppTask;
        return sAppTask;
    };
    CHIP_ERROR StartApp();

private:
    enum class OperatingMode : uint8_t
    {
        Normal,
        FactoryReset,
        Invalid
    };

    CHIP_ERROR Init();
    void DispatchEvent(AppEvent * aEvent);

    // statics needed to interact with zephyr C API
    static void CancelTimer(void);
    static void StartTimer(uint32_t aTimeoutInMs);
    static void FunctionTimerEventHandler(AppEvent * aEvent);
    static void FunctionHandler(AppEvent * aEvent);
    static void ButtonEventHandler(uint32_t aButtonsState, uint32_t aHasChanged);
    static void TimerEventHandler(k_timer * aTimer);
    static void PostEvent(AppEvent * aEvent);
    static void UpdateStatusLED();
    static void LEDStateUpdateHandler(LEDWidget & aLedWidget);
    static void UpdateLedStateEventHandler(AppEvent * aEvent);
    static void StartBLEAdvertisementHandler(AppEvent * aEvent);
    static void ChipEventHandler(const chip::DeviceLayer::ChipDeviceEvent * aEvent, intptr_t aArg);

    OperatingMode mMode{ OperatingMode::Normal };
    bool mFunctionTimerActive{ false };
    bool mIsThreadProvisioned{ false };
    bool mIsThreadEnabled{ false };
    bool mHaveBLEConnections{ false };

#if CONFIG_CHIP_FACTORY_DATA
    chip::DeviceLayer::FactoryDataProvider<chip::DeviceLayer::InternalFlashFactoryData> mFactoryDataProvider;
#endif
};
