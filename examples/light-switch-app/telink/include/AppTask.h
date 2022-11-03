/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "AppEvent.h"

#include <zephyr/drivers/gpio.h>

#include <platform/CHIPDeviceLayer.h>

#include <cstdint>

struct k_timer;

class AppTask
{
public:
    CHIP_ERROR StartApp();

    enum Action_t : uint8_t
    {
        ON_ACTION = 0,
        OFF_ACTION,
        LEVEL_ACTION,

        INVALID_ACTION
    };

    void PostLightingActionRequest(Action_t aAction);
    void PostEvent(AppEvent * event);
    void UpdateClusterState();

private:
    friend AppTask & GetAppTask(void);

    CHIP_ERROR Init();

    static void ActionInitiated(AppTask::Action_t aAction, int32_t aActor);
    static void ActionCompleted(AppTask::Action_t aAction, int32_t aActor);

    void DispatchEvent(AppEvent * event);

    static void UpdateStatusLED();
    static void SwitchActionButtonEventHandler(void);
    static void FactoryResetButtonEventHandler(void);
    static void StartThreadButtonEventHandler(void);
    static void StartBleAdvButtonEventHandler(void);

    static void ChipEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);

    static void FactoryResetHandler(AppEvent * aEvent);
    static void StartThreadHandler(AppEvent * aEvent);
    static void SwitchActionEventHandler(AppEvent * aEvent);
    static void StartBleAdvHandler(AppEvent * aEvent);

    static void InitButtons(void);

    static void ThreadProvisioningHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);

    static AppTask sAppTask;
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}
