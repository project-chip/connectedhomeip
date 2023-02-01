// refactored from the NRF lock-app

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
#include "BoltLockManager.h"
#include "LEDWidget.h"

#include <platform/CHIPDeviceLayer.h>

#if CONFIG_CHIP_FACTORY_DATA
#include <platform/nrfconnect/FactoryDataProvider.h>
#endif

// #ifdef CONFIG_MCUMGR_SMP_BT
// #include "DFUOverSMP.h"
// #endif

struct k_timer;
struct Identify;

class AppTask
{
public:
    static AppTask & Instance()
    {
        static AppTask sAppTask;
        return sAppTask;
    };

    CHIP_ERROR StartApp();

    void UpdateClusterState(BoltLockManager::State state, BoltLockManager::OperationSource source);

    // static void PostEvent(const AppEvent & event);
    static void PostEvent(AppEvent * event);

    static void IdentifyStartHandler(Identify *);
    static void IdentifyStopHandler(Identify *);

private:
    friend AppTask & GetAppTask(void);
    CHIP_ERROR Init();

    void CancelTimer();
    void StartTimer(uint32_t timeoutInMs);

    // static void DispatchEvent(const AppEvent & event);
    void DispatchEvent(AppEvent * event);
    static void FunctionTimerEventHandler(const AppEvent & event);
    static void FunctionHandler(const AppEvent & event);
    static void StartBLEAdvertisementAndLockActionEventHandler(const AppEvent & event);
    static void LockActionEventHandler(const AppEvent & event);
    static void StartBLEAdvertisementHandler(const AppEvent & event);
    // static void UpdateLedStateEventHandler(const AppEvent & event);

    // static void ChipEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);
    static void ButtonEventHandler(uint32_t buttonState, uint32_t hasChanged);
    // static void LEDStateUpdateHandler(LEDWidget & ledWidget);
    static void FunctionTimerTimeoutCallback(k_timer * timer);
    static void InitButtons(void);
    // static void StartBleAdvHandler(AppEvent * aEvent);


    static void UpdateStatusLED();
    static void LEDStateUpdateHandler(LEDWidget * ledWidget);
    static void SwitchActionButtonEventHandler(void);
    static void FactoryResetButtonEventHandler(void);
    static void StartThreadButtonEventHandler(void);
    static void StartBleAdvButtonEventHandler(void);

    static void ChipEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);

    static void FactoryResetTimerTimeoutCallback(k_timer * timer);

    static void FactoryResetTimerEventHandler(AppEvent * aEvent);
    static void FactoryResetHandler(AppEvent * aEvent);
    static void StartThreadHandler(AppEvent * aEvent);
    static void SwitchActionEventHandler(AppEvent * aEvent);
    static void StartBleAdvHandler(AppEvent * aEvent);
    static void UpdateLedStateEventHandler(AppEvent * aEvent);
    static void UpdateIdentifyStateEventHandler(AppEvent * aEvent);
    static void LockStateChanged(BoltLockManager::State state, BoltLockManager::OperationSource source);

// #ifdef CONFIG_MCUMGR_SMP_BT
//     static void RequestSMPAdvertisingStart(void);
// #endif

    // FunctionEvent mFunction   = FunctionEvent::NoneSelected;
    // bool mFunctionTimerActive = false;

#if CONFIG_CHIP_FACTORY_DATA
    chip::DeviceLayer::FactoryDataProvider<chip::DeviceLayer::InternalFlashFactoryData> mFactoryDataProvider;
#endif
    static AppTask sAppTask;
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}
