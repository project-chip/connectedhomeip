/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "AppEvent.h"

#include "FreeRTOS.h"
#include "timers.h" // provides FreeRTOS timer support
#include <ble/BLEEndPoint.h>
#include <platform/CHIPDeviceLayer.h>

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
    void PostEvent(const AppEvent * event);
    static void AppTaskMain(void * pvParameter);
    static void UpdateClusterState(void);
    static void ButtonEventHandler(uint8_t, uint8_t);
    static void UpdateStatusLED();

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
        FactoryReset
    };
    TimerFunction mFunction = TimerFunction::NoneSelected;

    enum class Button : uint8_t
    {
        Function,
        Dimmer,
    };

    friend AppTask & GetAppTask(void);
    static AppTask sAppTask;

    CHIP_ERROR Init();

    void DispatchEvent(AppEvent * event);

    static void ButtonPushHandler(AppEvent *);
    static void ButtonReleaseHandler(AppEvent *);
    static void StartBLEAdvertisingHandler(AppEvent *);

    static void ChipEventHandler(const chip::DeviceLayer::ChipDeviceEvent *, intptr_t);
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}
