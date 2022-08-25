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

#ifdef CONFIG_MCUMGR_SMP_BT
#include "DFUOverSMP.h"
#endif

struct k_timer;

class AppTask
{
public:
    CHIP_ERROR StartApp();

    void PostEvent(AppEvent * event);
    void UpdateClusterState(BoltLockManager::State state, BoltLockManager::OperationSource source);

private:
    friend AppTask & GetAppTask(void);

    CHIP_ERROR Init();

    static void LockStateChanged(BoltLockManager::State state, BoltLockManager::OperationSource source);

    void CancelTimer(void);

    void DispatchEvent(AppEvent * event);

    static void UpdateStatusLED();
    static void LEDStateUpdateHandler(LEDWidget & ledWidget);
    static void UpdateLedStateEventHandler(AppEvent * aEvent);
    static void FunctionTimerEventHandler(AppEvent * aEvent);
    static void FunctionHandler(AppEvent * aEvent);
    static void LockActionEventHandler(AppEvent * aEvent);
    static void StartBLEAdvertisementHandler(AppEvent * aEvent);

    static void ChipEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);

    static void ButtonEventHandler(uint32_t buttons_state, uint32_t has_changed);
    static void TimerEventHandler(k_timer * timer);

#ifdef CONFIG_MCUMGR_SMP_BT
    static void RequestSMPAdvertisingStart(void);
#endif

    void StartTimer(uint32_t aTimeoutInMs);

    enum Function_t
    {
        kFunction_NoneSelected   = 0,
        kFunction_SoftwareUpdate = 0,
        kFunction_FactoryReset,

        kFunction_Invalid
    };

    Function_t mFunction      = kFunction_NoneSelected;
    bool mFunctionTimerActive = false;
    static AppTask sAppTask;

#if CONFIG_CHIP_FACTORY_DATA
    chip::DeviceLayer::FactoryDataProvider<chip::DeviceLayer::InternalFlashFactoryData> mFactoryDataProvider;
#endif
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}
