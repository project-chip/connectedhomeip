/*
 *
 * SPDX-FileCopyrightText: 2020 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "AppEvent.h"
#include "BoltLockManager.h"

#include "CHIPProjectConfig.h"

#if CONFIG_CHIP_LOAD_REAL_FACTORY_DATA
#include <platform/nxp/k32w/k32w0/FactoryDataProviderImpl.h>
#if CHIP_DEVICE_CONFIG_USE_CUSTOM_PROVIDER
#include "CustomFactoryDataProvider.h"
#endif
#endif

#include <platform/CHIPDeviceLayer.h>

#include "FreeRTOS.h"
#include "timers.h"

class AppTask
{
public:
#if CONFIG_CHIP_LOAD_REAL_FACTORY_DATA
#if CHIP_DEVICE_CONFIG_USE_CUSTOM_PROVIDER
    using FactoryDataProvider = chip::DeviceLayer::CustomFactoryDataProvider;
#else
    using FactoryDataProvider = chip::DeviceLayer::FactoryDataProviderImpl;
#endif
#endif

public:
    CHIP_ERROR StartAppTask();
    static void AppTaskMain(void * pvParameter);

    void PostLockActionRequest(int32_t aActor, BoltLockManager::Action_t aAction);
    void PostEvent(const AppEvent * event);

    void UpdateClusterState(void);

private:
    friend AppTask & GetAppTask(void);

    CHIP_ERROR Init();

    static void ActionInitiated(BoltLockManager::Action_t aAction, int32_t aActor);
    static void ActionCompleted(BoltLockManager::Action_t aAction);

    void CancelTimer(void);

    void DispatchEvent(AppEvent * event);

    static void FunctionTimerEventHandler(void * aGenericEvent);
    static void KBD_Callback(uint8_t events);
    static void HandleKeyboard(void);
    static void JoinHandler(void * aGenericEvent);
    static void BleHandler(void * aGenericEvent);
    static void BleStartAdvertising(intptr_t arg);
    static void LockActionEventHandler(void * aGenericEvent);
    static void ResetActionEventHandler(void * aGenericEvent);
    static void InstallEventHandler(void * aGenericEvent);

    static void ButtonEventHandler(uint8_t pin_no, uint8_t button_action);
    static void TimerEventHandler(TimerHandle_t xTimer);

    static void MatterEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);
    static void UpdateClusterStateInternal(intptr_t arg);
    static void ThreadStart();
    static void InitServer(intptr_t arg);
    static void PrintOnboardingInfo();
    void StartTimer(uint32_t aTimeoutInMs);

    enum Function_t
    {
        kFunction_NoneSelected   = 0,
        kFunction_SoftwareUpdate = 0,
        kFunction_FactoryReset,
        kFunctionLockUnlock,

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
