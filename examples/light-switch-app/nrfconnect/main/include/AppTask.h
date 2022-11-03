/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "AppEvent.h"
#include "LEDWidget.h"

#include <core/CHIPError.h>
#include <platform/CHIPDeviceLayer.h>

#if CONFIG_CHIP_FACTORY_DATA
#include <platform/nrfconnect/FactoryDataProvider.h>
#endif

#ifdef CONFIG_MCUMGR_SMP_BT
#include "DFUOverSMP.h"
#endif

#include <cstdint>

struct k_timer;
struct Identify;

class AppTask
{
public:
    CHIP_ERROR StartApp();
    void PostEvent(AppEvent *);
    void UpdateClusterState();
    static void IdentifyStartHandler(Identify *);
    static void IdentifyStopHandler(Identify *);

private:
    enum class Timer : uint8_t
    {
        Function,
        DimmerTrigger,
        Dimmer
    };
    enum class TimerFunction : uint8_t
    {
        NoneSelected = 0,
        SoftwareUpdate,
        FactoryReset,
    };
    TimerFunction mFunction = TimerFunction::NoneSelected;

    enum class Button : uint8_t
    {
        Function,
        Dimmer,
    };

    friend AppTask & GetAppTask();
    static AppTask sAppTask;

    CHIP_ERROR Init();

    void DispatchEvent(AppEvent *);

    static void ButtonPushHandler(AppEvent *);
    static void ButtonReleaseHandler(AppEvent *);
    static void TimerEventHandler(AppEvent *);
    static void StartBLEAdvertisingHandler(AppEvent *);
    static void UpdateLedStateEventHandler(AppEvent *);

    static void ChipEventHandler(const chip::DeviceLayer::ChipDeviceEvent *, intptr_t);
    static void UpdateStatusLED();
    static void ButtonEventHandler(uint32_t, uint32_t);
    static void LEDStateUpdateHandler(LEDWidget &);

    static void StartTimer(Timer, uint32_t);
    static void CancelTimer(Timer);
    static void TimerEventHandler(k_timer *);

#ifdef CONFIG_MCUMGR_SMP_BT
    static void RequestSMPAdvertisingStart(void);
#endif

#if CONFIG_CHIP_FACTORY_DATA
    chip::DeviceLayer::FactoryDataProvider<chip::DeviceLayer::InternalFlashFactoryData> mFactoryDataProvider;
#endif
};

inline AppTask & GetAppTask()
{
    return AppTask::sAppTask;
}
