/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: (c) 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "AppEvent.h"
#include "LEDWidget.h"
#include "PumpManager.h"

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

    void PostStartActionRequest(int32_t aActor, PumpManager::Action_t aAction);
    void PostEvent(AppEvent * event);
    void UpdateClusterState();

private:
    friend AppTask & GetAppTask(void);

    CHIP_ERROR Init();

    static void ActionInitiated(PumpManager::Action_t aAction, int32_t aActor);
    static void ActionCompleted(PumpManager::Action_t aAction, int32_t aActor);

    void CancelTimer(void);

    void DispatchEvent(AppEvent * event);

    static void UpdateStatusLED();
    static void LEDStateUpdateHandler(LEDWidget & ledWidget);
    static void UpdateLedStateEventHandler(AppEvent * aEvent);
    static void FunctionTimerEventHandler(AppEvent * aEvent);
    static void FunctionHandler(AppEvent * aEvent);
    static void StartActionEventHandler(AppEvent * aEvent);
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
