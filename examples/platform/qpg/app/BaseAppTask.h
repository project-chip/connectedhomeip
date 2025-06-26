/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**********************************************************
 * Includes
 *********************************************************/

#include <stdbool.h>
#include <stdint.h>

#include "AppEvent.h"

#include "FreeRTOS.h"
#include "timers.h" // provides FreeRTOS timer support
#include <platform/CHIPDeviceLayer.h>

#include <platform/qpg/FactoryDataProvider.h>

/**********************************************************
 * Defines
 *********************************************************/

#define APP_NAME "Base-Matter-app"

// Application-defined error codes in the CHIP_ERROR space.
#define APP_ERROR_EVENT_QUEUE_FAILED CHIP_APPLICATION_ERROR(0x01)
#define APP_ERROR_CREATE_TASK_FAILED CHIP_APPLICATION_ERROR(0x02)
#define APP_ERROR_UNHANDLED_EVENT CHIP_APPLICATION_ERROR(0x03)
#define APP_ERROR_CREATE_TIMER_FAILED CHIP_APPLICATION_ERROR(0x04)
#define APP_ERROR_START_TIMER_FAILED CHIP_APPLICATION_ERROR(0x05)
#define APP_ERROR_STOP_TIMER_FAILED CHIP_APPLICATION_ERROR(0x06)

#define FACTORY_RESET_TRIGGER_TIMEOUT_MS (3000)
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT_MS (3000)
#define OTA_START_TRIGGER_TIMEOUT_MS (1500)
#ifdef GP_UPGRADE_DIVERSITY_DUAL_BOOT
#define STACK_SWITCH_CANCEL_WINDOW_TIMEOUT_MS (3000)
#endif // GP_UPGRADE_DIVERSITY_DUAL_BOOT

#define APP_TASK_NAME "APP"
#define APP_TASK_STACK_SIZE (3 * 1024)
#define APP_TASK_PRIORITY 2
#define APP_EVENT_QUEUE_SIZE 10

#define ONE_HOUR_SEC (3600)
#define ONE_SECOND_US (1000000UL)
#define TOTAL_OPERATIONAL_HOURS_SAVE_INTERVAL_SEC (1 * ONE_HOUR_SEC) // increment every hour

#define NMBR_OF_RESETS_BLE_ADVERTISING (3)

/**********************************************************
 * AppTask Declaration
 *********************************************************/

class BaseAppTask
{

public:
    virtual ~BaseAppTask() = default;
    virtual CHIP_ERROR Init();
    virtual void InitServer(intptr_t arg);
    virtual void UpdateClusterState() {}

    virtual bool ButtonEventHandler(uint8_t btnIdx, bool btnPressed);
    virtual void PowerCycleExpiredHandler(uint8_t resetCounts) {}

    CHIP_ERROR StartAppTask();
    void PostEvent(const AppEvent * event);

    static void AppTaskMain(void * pvParameter);
    static BaseAppTask & GetAppTask(void) { return sAppTask; }

    // Static wrapper that redirects to the instance method
    static void InitServerWrapper(intptr_t arg) { sAppTask.InitServer(arg); }
    static void ButtonEventHandlerWrapper(uint8_t button, bool pressed) { sAppTask.ButtonEventHandler(button, pressed); }
    static void PowerCycleExpiredHandlerWrapper(uint8_t resetCounts) { sAppTask.PowerCycleExpiredHandler(resetCounts); }

protected:
    static void InitDiagnosticLogs();
    static void OpenCommissioning(intptr_t arg);
    static void UpdateLEDs();

    static void FunctionHandler(AppEvent * aEvent);
    static void FunctionTimerEventHandler(AppEvent * aEvent);
    static void TimerEventHandler(chip::System::Layer * aLayer, void * aAppState);
    static void MatterEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);
    static void TotalHoursTimerHandler(chip::System::Layer * aLayer, void * aAppState);

    void StartTimer(uint32_t aTimeoutMs);
    void CancelTimer();

    void DispatchEvent(AppEvent * event);

    enum BaseFunction_t
    {
        kFunction_NoneSelected    = 0,
        kFunction_SoftwareUpdate  = 1,
        kFunction_StartBleAdv     = 2,
        kFunction_PreFactoryReset = 3,
        kFunction_FactoryReset    = 4,
        kFunction_PreStackSwitch  = 5,
        kFunction_StackSwitch     = 6,

        kFunction_Invalid = 255
    } BaseFunction_;

    BaseFunction_t mFunction        = kFunction_NoneSelected;
    bool mFunctionTimerActive       = false;
    bool mSyncClusterToButtonAction = false;

    chip::DeviceLayer::FactoryDataProvider mFactoryDataProvider;

    static BaseAppTask sAppTask;
};
