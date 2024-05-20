/*
 *
 *    Copyright (c) 2022 Google LLC.
 *    Copyright (c) 2023 NXP
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
#include "ContactSensorManager.h"

#include "CHIPProjectConfig.h"

#include <app/clusters/identify-server/identify-server.h>
#include <platform/CHIPDeviceLayer.h>

#if CONFIG_CHIP_LOAD_REAL_FACTORY_DATA
#include <platform/nxp/k32w/k32w1/FactoryDataProviderImpl.h>
#endif

#include "FreeRTOS.h"
#include "fsl_component_button.h"
#include "fsl_pm_core.h"
#include "timers.h"

// Application-defined error codes in the CHIP_ERROR space.
#define APP_ERROR_EVENT_QUEUE_FAILED CHIP_APPLICATION_ERROR(0x01)
#define APP_ERROR_CREATE_TASK_FAILED CHIP_APPLICATION_ERROR(0x02)
#define APP_ERROR_UNHANDLED_EVENT CHIP_APPLICATION_ERROR(0x03)
#define APP_ERROR_CREATE_TIMER_FAILED CHIP_APPLICATION_ERROR(0x04)
#define APP_ERROR_START_TIMER_FAILED CHIP_APPLICATION_ERROR(0x05)
#define APP_ERROR_STOP_TIMER_FAILED CHIP_APPLICATION_ERROR(0x06)
#define APP_ERROR_PM_REGISTER_LP_CALLBACK_FAILED CHIP_APPLICATION_ERROR(0x07)

class AppTask
{
public:
#if CONFIG_CHIP_LOAD_REAL_FACTORY_DATA
    using FactoryDataProvider = chip::DeviceLayer::FactoryDataProviderImpl;
#endif
    CHIP_ERROR StartAppTask();
    static void AppTaskMain(void * pvParameter);

    void PostContactActionRequest(ContactSensorManager::Action aAction);
    void PostEvent(const AppEvent * event);

    void UpdateClusterState(void);
    void UpdateDeviceState(void);

    bool IsSyncClusterToButtonAction();
    void SetSyncClusterToButtonAction(bool value);
    // Identify cluster callbacks.
    static void OnIdentifyStart(Identify * identify);
    static void OnIdentifyStop(Identify * identify);

    static status_t LowPowerCallback(pm_event_type_t eventType, uint8_t powerState, void * data);

private:
    friend AppTask & GetAppTask(void);

    CHIP_ERROR Init();

    static void OnStateChanged(ContactSensorManager::State aState);

    void CancelTimer(void);

    void DispatchEvent(AppEvent * event);

    static void FunctionTimerEventHandler(void * aGenericEvent);
    static button_status_t KBD_Callback(void * buttonHandle, button_callback_message_t * message, void * callbackParam);
    static void HandleKeyboard(void);
    static void SoftResetHandler(void * aGenericEvent);
    static void BleHandler(void * aGenericEvent);
    static void BleStartAdvertising(intptr_t arg);
    static void ContactActionEventHandler(void * aGenericEvent);
    static void ResetActionEventHandler(void * aGenericEvent);
    static void InstallEventHandler(void * aGenericEvent);
#if CHIP_ENABLE_LIT
    static void UserActiveModeHandler(void * aGenericEvent);
    static void UserActiveModeTrigger(intptr_t arg);
#endif

    static void ButtonEventHandler(uint8_t pin_no, uint8_t button_action);
    static void TimerEventHandler(TimerHandle_t xTimer);

    static void MatterEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);

    void StartTimer(uint32_t aTimeoutInMs);

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    static void InitOTA(intptr_t arg);
#endif

    static void UpdateClusterStateInternal(intptr_t arg);
    static void UpdateDeviceStateInternal(intptr_t arg);
    static void InitServer(intptr_t arg);
    static void PrintOnboardingInfo();

    enum class Function : uint8_t
    {
        kNoneSelected = 0,
        kFactoryReset,
        kContact,
        kIdentify,
        kInvalid
    };

    Function mFunction              = Function::kNoneSelected;
    bool mResetTimerActive          = false;
    bool mSyncClusterToButtonAction = false;

    static AppTask sAppTask;
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
