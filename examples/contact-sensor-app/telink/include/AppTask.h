/*
 *
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
#include "ContactSensorManager.h"
#include "LEDWidget.h"

#include <zephyr/drivers/gpio.h>

#include <platform/CHIPDeviceLayer.h>

#if CONFIG_CHIP_FACTORY_DATA
#include <platform/telink/FactoryDataProvider.h>
#endif

#include <cstdint>

// Application-defined error codes in the CHIP_ERROR space.
#define APP_ERROR_UNHANDLED_EVENT CHIP_APPLICATION_ERROR(0x03)

struct k_timer;

class AppTask
{
public:
    CHIP_ERROR StartApp(void);

    void PostContactActionRequest(ContactSensorManager::Action aAction);
    void PostEvent(AppEvent * event);
    void UpdateClusterState(void);
    void UpdateDeviceState(void);

    bool IsSyncClusterToButtonAction(void);
    void SetSyncClusterToButtonAction(bool value);

private:
    friend AppTask & GetAppTask(void);
    CHIP_ERROR Init(void);

    void DispatchEvent(AppEvent * event);

    static void OnStateChanged(ContactSensorManager::State aState);

    static void UpdateClusterStateInternal(intptr_t arg);
    static void UpdateDeviceStateInternal(intptr_t arg);

    static void UpdateStatusLED(void);
    static void LEDStateUpdateHandler(LEDWidget * ledWidget);
    static void FactoryResetButtonEventHandler(void);
    static void StartBleAdvButtonEventHandler(void);
    static void ToggleContactStateButtonEventHandler(void);

    static void ChipEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);

    static void FactoryResetTimerTimeoutCallback(k_timer * timer);

    static void FactoryResetTimerEventHandler(AppEvent * aEvent);
    static void FactoryResetHandler(AppEvent * aEvent);
    static void StartBleAdvHandler(AppEvent * aEvent);
    static void UpdateLedStateEventHandler(AppEvent * aEvent);
    static void ContactActionEventHandler(AppEvent * aEvent);

    static void InitButtons(void);

    static void ThreadProvisioningHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);

    bool mSyncClusterToButtonAction = false;

    static AppTask sAppTask;

#if CONFIG_CHIP_FACTORY_DATA
    // chip::DeviceLayer::FactoryDataProvider<chip::DeviceLayer::InternalFlashFactoryData> mFactoryDataProvider;
    chip::DeviceLayer::FactoryDataProvider<chip::DeviceLayer::ExternalFlashFactoryData> mFactoryDataProvider;
#endif
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}

inline bool AppTask::IsSyncClusterToButtonAction()
{
    return mSyncClusterToButtonAction;
}

inline void AppTask::SetSyncClusterToButtonAction(bool value)
{
    mSyncClusterToButtonAction = value;
}
