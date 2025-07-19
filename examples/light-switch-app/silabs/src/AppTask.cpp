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

/**********************************************************
 * Includes
 *********************************************************/

#include "AppTask.h"
#include "AppConfig.h"
#include "AppEvent.h"
#include "BindingHandler.h"
#include "LEDWidget.h"
#include "LightSwitchMgr.h"
#ifdef DISPLAY_ENABLED
#include "lcd.h"
#ifdef QR_CODE_ENABLED
#include "qrcodegen.h"
#endif // QR_CODE_ENABLED
#endif // DISPLAY_ENABLED
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <assert.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/silabs/platformAbstraction/SilabsPlatform.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

/**********************************************************
 * Defines and Constants
 *********************************************************/

#define SYSTEM_STATE_LED &sl_led_led0

namespace {
constexpr chip::EndpointId kLightSwitchEndpoint   = 1;
constexpr chip::EndpointId kGenericSwitchEndpoint = 2;
} // namespace

using namespace chip;
using namespace chip::app;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Silabs;

using namespace chip::TLV;
using namespace ::chip::DeviceLayer;

/**********************************************************
 * AppTask Definitions
 *********************************************************/

AppTask AppTask::sAppTask;

bool AppTask::functionButtonPressed  = false;
bool AppTask::actionButtonPressed    = false;
bool AppTask::actionButtonSuppressed = false;
bool AppTask::isButtonEventTriggered = false;

CHIP_ERROR AppTask::AppInit()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::DeviceLayer::Silabs::GetPlatform().SetButtonsCb(AppTask::ButtonEventHandler);

    err = LightSwitchMgr::GetInstance().Init(kLightSwitchEndpoint, kGenericSwitchEndpoint);
    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("LightSwitchMgr Init failed!");
        appError(err);
    }

    longPressTimer = new Timer(LONG_PRESS_TIMEOUT_MS, OnLongPressTimeout, this);

    return err;
}

void AppTask::Timer::Start()
{
    // Starts or restarts the function timer
    osStatus_t status = osTimerStart(mHandler, pdMS_TO_TICKS(LONG_PRESS_TIMEOUT_MS));
    if (status != osOK)
    {
        SILABS_LOG("Timer start() failed with error code : %lx", status);
        appError(APP_ERROR_START_TIMER_FAILED);
    }

    mIsActive = true;
}

void AppTask::Timer::Timeout()
{
    mIsActive = false;
    if (mCallback)
    {
        mCallback(*this);
    }
}

void AppTask::HandleLongPress()
{
    AppEvent event;
    event.Handler = AppTask::AppEventHandler;

    if (actionButtonPressed)
    {
        actionButtonSuppressed = true;
        // Long press button up : Trigger Level Control Action
        event.Type = AppEvent::kEventType_TriggerLevelControlAction;
        AppTask::GetAppTask().PostEvent(&event);
    }
}

void AppTask::OnLongPressTimeout(AppTask::Timer & timer)
{
    AppTask * app = static_cast<AppTask *>(timer.mContext);
    if (app)
    {
        app->HandleLongPress();
    }
}

AppTask::Timer::Timer(uint32_t timeoutInMs, Callback callback, void * context) : mCallback(callback), mContext(context)
{
    mHandler = osTimerNew(TimerCallback, // timer callback handler
                          osTimerOnce,   // no timer reload (one-shot timer)
                          this,          // pass the app task obj context
                          NULL           // No osTimerAttr_t to provide.
    );

    if (mHandler == NULL)
    {
        SILABS_LOG("Timer create failed");
        appError(APP_ERROR_CREATE_TIMER_FAILED);
    }
}

AppTask::Timer::~Timer()
{
    if (mHandler)
    {
        osTimerDelete(mHandler);
        mHandler = nullptr;
    }
}

void AppTask::Timer::Stop()
{
    // Abort on osError (-1) as it indicates an unspecified failure with no clear recovery path.
    if (osTimerStop(mHandler) == osError)
    {
        SILABS_LOG("Timer stop() failed");
        appError(APP_ERROR_STOP_TIMER_FAILED);
    }
    mIsActive = false;
}

void AppTask::Timer::TimerCallback(void * timerCbArg)
{
    Timer * timer = reinterpret_cast<Timer *>(timerCbArg);
    if (timer)
    {
        timer->Timeout();
    }
}

CHIP_ERROR AppTask::StartAppTask()
{
    return BaseApplication::StartAppTask(AppTaskMain);
}

void AppTask::AppTaskMain(void * pvParameter)
{
    AppEvent event;
    osMessageQueueId_t sAppEventQueue = *(static_cast<osMessageQueueId_t *>(pvParameter));

    CHIP_ERROR err = sAppTask.Init();
    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("AppTask.Init() failed");
        appError(err);
    }

#if !(defined(CHIP_CONFIG_ENABLE_ICD_SERVER) && CHIP_CONFIG_ENABLE_ICD_SERVER)
    sAppTask.StartStatusLEDTimer();
#endif

    SILABS_LOG("App Task started");
    while (true)
    {
        osStatus_t eventReceived = osMessageQueueGet(sAppEventQueue, &event, NULL, osWaitForever);
        while (eventReceived == osOK)
        {
            sAppTask.DispatchEvent(&event);
            eventReceived = osMessageQueueGet(sAppEventQueue, &event, NULL, 0);
        }
    }
}

void AppTask::ButtonEventHandler(uint8_t button, uint8_t btnAction)
{
    AppEvent event = {};
    event.Handler  = AppTask::AppEventHandler;
    if (btnAction == to_underlying(SilabsPlatform::ButtonAction::ButtonPressed))
    {
        event.Type = (button ? AppEvent::kEventType_ActionButtonPressed : AppEvent::kEventType_FunctionButtonPressed);
    }
    else
    {
        event.Type = (button ? AppEvent::kEventType_ActionButtonReleased : AppEvent::kEventType_FunctionButtonReleased);
    }
    AppTask::GetAppTask().PostEvent(&event);
}

void AppTask::AppEventHandler(AppEvent * aEvent)
{
    switch (aEvent->Type)
    {
    case AppEvent::kEventType_FunctionButtonPressed:
        functionButtonPressed = true;
        if (actionButtonPressed)
        {
            actionButtonSuppressed = true;
            LightSwitchMgr::GetInstance().changeStepMode();
        }
        else
        {
            isButtonEventTriggered = true;
            // Post button press event to BaseApplication
            AppEvent button_event           = {};
            button_event.Type               = AppEvent::kEventType_Button;
            button_event.ButtonEvent.Action = static_cast<uint8_t>(SilabsPlatform::ButtonAction::ButtonPressed);
            button_event.Handler            = BaseApplication::ButtonHandler;
            AppTask::GetAppTask().PostEvent(&button_event);
        }
        break;
    case AppEvent::kEventType_FunctionButtonReleased: {
        functionButtonPressed = false;
        if (isButtonEventTriggered)
        {
            isButtonEventTriggered = false;
            // Post button release event to BaseApplication
            AppEvent button_event           = {};
            button_event.Type               = AppEvent::kEventType_Button;
            button_event.ButtonEvent.Action = static_cast<uint8_t>(SilabsPlatform::ButtonAction::ButtonReleased);
            button_event.Handler            = BaseApplication::ButtonHandler;
            AppTask::GetAppTask().PostEvent(&button_event);
        }
        break;
    }
    case AppEvent::kEventType_ActionButtonPressed:
        actionButtonPressed = true;
        LightSwitchMgr::GetInstance().SwitchActionEventHandler(aEvent->Type);
        if (functionButtonPressed)
        {
            actionButtonSuppressed = true;
            LightSwitchMgr::GetInstance().changeStepMode();
        }
        else if (sAppTask.longPressTimer)
        {
            sAppTask.longPressTimer->Start();
        }
        break;
    case AppEvent::kEventType_ActionButtonReleased:
        actionButtonPressed = false;
        if (sAppTask.longPressTimer)
        {
            sAppTask.longPressTimer->Stop();
        }
        if (actionButtonSuppressed)
        {
            actionButtonSuppressed = false;
        }
        else
        {
            aEvent->Type = AppEvent::kEventType_TriggerToggle;
            LightSwitchMgr::GetInstance().SwitchActionEventHandler(aEvent->Type);
        }
        aEvent->Type = AppEvent::kEventType_ActionButtonReleased;
        LightSwitchMgr::GetInstance().SwitchActionEventHandler(aEvent->Type);
        break;
    case AppEvent::kEventType_TriggerLevelControlAction:
        LightSwitchMgr::GetInstance().SwitchActionEventHandler(aEvent->Type);
    default:
        break;
    }
}
