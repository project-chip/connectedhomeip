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

#include <AppEvent.h>
#include <AppTimer.h>
#include <ButtonHandler.h>
#include <FreeRTOS.h>
#include <LEDWidget.h>
#include <WindowCover.h>
#include <ble/BLEEndPoint.h>
#include <platform/CHIPDeviceLayer.h>
#include <queue.h>
#include <stdbool.h>
#include <stdint.h>
#include <string>

using namespace ::chip::DeviceLayer;

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
    typedef enum EventSource
    {
        kEventSource_None = 0,
        kEventSource_WindowCover,
        kEventSource_ButtonManager,
        kEventSource_ZCL,
    } EventSource;

    static AppTask & Instance();

    CHIP_ERROR Start(void);
    WindowCover & Cover();
    void PostEvent(const AppEvent & event);

private:
    AppTask() = default;
    static AppTask sInstance;

    static void Main(void * pvParameter);
    static void IconTimerCallback(AppTimer & timer, void * context);
    static void CoverTypeTimerCallback(AppTimer & timer, void * context);
    static void ResetTimerCallback(AppTimer & timer, void * context);

    bool mIsThreadProvisioned     = false;
    bool mLastThreadProvisioned   = false;
    bool mIsThreadEnabled         = false;
    bool mHaveBLEConnections      = false;
    bool mHaveServiceConnectivity = false;
    TaskHandle_t mHandle          = nullptr;
    LEDWidget mStatusLED;
    LEDWidget mActionLED;
    AppTimer mCoverTypeTimer;
    AppTimer mIconTimer;
    AppTimer mResetTimer;
    QueueHandle_t mQueue = nullptr;
    WindowCover mCover;
    std::string mQRCode;
    bool mResetWarning;

    int Init(void);
    void DispatchEvents(void);
    void DispatchButtonEvent(AppEvent::EventType type, void * context);
    void DispatchWindowCoverEvent(AppEvent::EventType type, void * context);
    void UpdateLog(AppEvent::EventType event);
    void UpdateLed(AppEvent::EventType event);
    void UpdateLcd(AppEvent::EventType event);
    void UpdateClusterState(AppEvent::EventType event);
};
