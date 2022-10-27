/*
 *
 *    Copyright (c) 2021 Google LLC.
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
#include "K32W0FactoryDataProvider.h"
#include "LightingManager.h"

#include <app/clusters/identify-server/identify-server.h>
#include <platform/CHIPDeviceLayer.h>

#include "FreeRTOS.h"
#include "timers.h"

// Application-defined error codes in the CHIP_ERROR space.
#define APP_ERROR_EVENT_QUEUE_FAILED CHIP_APPLICATION_ERROR(0x01)
#define APP_ERROR_CREATE_TASK_FAILED CHIP_APPLICATION_ERROR(0x02)
#define APP_ERROR_UNHANDLED_EVENT CHIP_APPLICATION_ERROR(0x03)
#define APP_ERROR_CREATE_TIMER_FAILED CHIP_APPLICATION_ERROR(0x04)
#define APP_ERROR_START_TIMER_FAILED CHIP_APPLICATION_ERROR(0x05)
#define APP_ERROR_STOP_TIMER_FAILED CHIP_APPLICATION_ERROR(0x06)

class AppTask
{
public:
    CHIP_ERROR StartAppTask();
    static void AppTaskMain(void * pvParameter);

    void PostTurnOnActionRequest(int32_t aActor, LightingManager::Action_t aAction);
    void PostEvent(const AppEvent * event);

    void UpdateClusterState(void);
    void UpdateDeviceState(void);

    // Identify cluster callbacks.
    static void OnIdentifyStart(Identify * identify);
    static void OnIdentifyStop(Identify * identify);
    static void OnTriggerEffect(Identify * identify);
    static void OnTriggerEffectComplete(chip::System::Layer * systemLayer, void * appState);

private:
    friend AppTask & GetAppTask(void);

    CHIP_ERROR Init();

    static void ActionInitiated(LightingManager::Action_t aAction, int32_t aActor);
    static void ActionCompleted(LightingManager::Action_t aAction);

    void CancelTimer(void);

    void DispatchEvent(AppEvent * event);

    static void FunctionTimerEventHandler(AppEvent * aEvent);
    static void KBD_Callback(uint8_t events);
    static void HandleKeyboard(void);
    static void OTAHandler(AppEvent * aEvent);
    static void BleHandler(AppEvent * aEvent);
    static void BleStartAdvertising(intptr_t arg);
    static void LightActionEventHandler(AppEvent * aEvent);
    static void OTAResumeEventHandler(AppEvent * aEvent);
    static void ResetActionEventHandler(AppEvent * aEvent);
    static void InstallEventHandler(AppEvent * aEvent);

    static void ButtonEventHandler(uint8_t pin_no, uint8_t button_action);
    static void TimerEventHandler(TimerHandle_t xTimer);

    static void MatterEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);
    void StartTimer(uint32_t aTimeoutInMs);

    static void RestoreLightingState(void);

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    static void InitOTA(intptr_t arg);
    static void StartOTAQuery(intptr_t arg);
    static void PostOTAResume();
    static void OnScheduleInitOTA(chip::System::Layer * systemLayer, void * appState);
#endif

    static void UpdateClusterStateInternal(intptr_t arg);
    static void UpdateDeviceStateInternal(intptr_t arg);
    static void InitServer(intptr_t arg);

    enum Function_t
    {
        kFunction_NoneSelected   = 0,
        kFunction_SoftwareUpdate = 0,
        kFunction_FactoryReset,
        kFunctionTurnOnTurnOff,
        kFunction_Identify,
        kFunction_TriggerEffect,

        kFunction_Invalid
    } Function;

    Function_t mFunction            = kFunction_NoneSelected;
    bool mResetTimerActive          = false;
    bool mSyncClusterToButtonAction = false;

    static AppTask sAppTask;
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}
