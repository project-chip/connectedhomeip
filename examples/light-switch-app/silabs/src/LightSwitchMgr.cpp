/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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

#include "LightSwitchMgr.h"
#include "BindingHandler.h"

#if defined(ENABLE_CHIP_SHELL)
#include "ShellCommands.h"
#endif // defined(ENABLE_CHIP_SHELL)

#include "AppConfig.h"
#include "AppEvent.h"
#include "AppTask.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/switch-server/switch-server.h>
#include <platform/CHIPDeviceLayer.h>
#include <cmsis_os2.h>
#include <platform/silabs/platformAbstraction/SilabsPlatform.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::DeviceLayer;
using namespace chip::DeviceLayer::Silabs;

LightSwitchMgr LightSwitchMgr::sSwitch;

AppEvent LightSwitchMgr::CreateNewEvent(AppEvent::AppEventTypes type)
{
    AppEvent aEvent;
    aEvent.Type                      = type;
    aEvent.Handler                   = LightSwitchMgr::AppEventHandler;
    LightSwitchMgr * lightSwitch         = &LightSwitchMgr::GetInstance();
    aEvent.LightSwitchEvent.Context  = lightSwitch;
    return aEvent;
}

void LightSwitchMgr::Timer::Start()
{
    // Starts or restarts the function timer
    if (osTimerStart(mHandler, pdMS_TO_TICKS(LONG_PRESS_TIMEOUT)) != osOK)
    {
        SILABS_LOG("Timer start() failed");
        appError(CHIP_ERROR_INTERNAL);
    }

    mIsActive = true;
}

void LightSwitchMgr::Timer::Timeout()
{
    mIsActive = false;
    if (mCallback)
    {
        mCallback(*this);
    }
}

void LightSwitchMgr::HandleLongPress()
{
    AppEvent event;
    event.Handler             = AppEventHandler;
    LightSwitchMgr * lightSwitch  = &LightSwitchMgr::GetInstance();
    event.LightSwitchEvent.Context  = lightSwitch;
    if (mFunctionButtonPressed)
    {
        if (!mResetWarning)
        {
            // Long press button down: Reset warning!
            event.Type = AppEvent::kEventType_ResetWarning;
            AppTask::GetAppTask().PostEvent(&event);
        }
    }
    else if (mActionButtonPressed)
    {
        mActionButtonSuppressed = true;
        // Long press button up : Trigger Level Control Action
        event.Type = AppEvent::kEventType_TriggerLevelControlAction;
        AppTask::GetAppTask().PostEvent(&event);
    }
}

void LightSwitchMgr::OnLongPressTimeout(LightSwitchMgr::Timer & timer)
{
    LightSwitchMgr * app = static_cast<LightSwitchMgr *>(timer.mContext);
    if (app)
    {
        app->HandleLongPress();
    }
}

LightSwitchMgr::Timer::Timer(uint32_t timeoutInMs, Callback callback, void * context) : mCallback(callback), mContext(context)
{
    mHandler = osTimerNew(TimerCallback, // timer callback handler
                          osTimerOnce,   // no timer reload (one-shot timer)
                          this,          // pass the app task obj context
                          NULL           // No osTimerAttr_t to provide.
    );
    
    if (mHandler == NULL)
    {
        SILABS_LOG("Timer create failed");
        appError(CHIP_ERROR_INTERNAL);
    }
}

LightSwitchMgr::Timer::~Timer()
{
    if (mHandler)
    {
        osTimerDelete(mHandler);
        mHandler = nullptr;
    }
}

void LightSwitchMgr::Timer::Stop()
{
    mIsActive = false;
    if (osTimerStop(mHandler) == osError)
    {
        SILABS_LOG("Timer stop() failed");
        appError(CHIP_ERROR_INTERNAL);
    }
}

void LightSwitchMgr::Timer::TimerCallback(void * timerCbArg)
{
    Timer * timer = reinterpret_cast<Timer *>(timerCbArg);
    if (timer)
    {
        timer->Timeout();
    }
}

/**
 * @brief Configures LightSwitchMgr
 *        This function needs to be call before using the LightSwitchMgr
 *
 * @param lightSwitchEndpoint endpoint for the light switch device type
 * @param genericSwitchEndpoint endpoint for the generic switch device type
 */
CHIP_ERROR LightSwitchMgr::Init(EndpointId lightSwitchEndpoint, chip::EndpointId genericSwitchEndpoint)
{
    VerifyOrReturnError(lightSwitchEndpoint != kInvalidEndpointId, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(genericSwitchEndpoint != kInvalidEndpointId, CHIP_ERROR_INVALID_ARGUMENT);

    mLightSwitchEndpoint   = lightSwitchEndpoint;
    mGenericSwitchEndpoint = genericSwitchEndpoint;

    mLongPressTimer = new Timer(LONG_PRESS_TIMEOUT, OnLongPressTimeout, this);

    // Configure Bindings
    CHIP_ERROR error = InitBindingHandler();
    if (error != CHIP_NO_ERROR)
    {
        SILABS_LOG("InitBindingHandler() failed!");
        appError(error);
    }

#if defined(ENABLE_CHIP_SHELL)
    LightSwitchCommands::RegisterSwitchCommands();
#endif // defined(ENABLE_CHIP_SHELL)

    return error;
}

/**
 * @brief Function that triggers a generic switch OnInitialPress event
 */
void LightSwitchMgr::GenericSwitchOnInitialPress()
{
    GenericSwitchEventData * data = Platform::New<GenericSwitchEventData>();

    data->endpoint = mGenericSwitchEndpoint;
    data->event    = Switch::Events::InitialPress::Id;

    DeviceLayer::PlatformMgr().ScheduleWork(GenericSwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
}

/**
 * @brief Function that triggers a generic switch OnShortRelease event
 */
void LightSwitchMgr::GenericSwitchOnShortRelease()
{
    GenericSwitchEventData * data = Platform::New<GenericSwitchEventData>();

    data->endpoint = mGenericSwitchEndpoint;
    data->event    = Switch::Events::ShortRelease::Id;

    DeviceLayer::PlatformMgr().ScheduleWork(GenericSwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
}

StepModeEnum LightSwitchMgr::getStepMode(){
    return stepDirection;
}

void LightSwitchMgr::TriggerLightSwitchAction(LightSwitchAction action, bool isGroupCommand)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();

    data->clusterId = chip::app::Clusters::OnOff::Id;
    data->isGroup   = isGroupCommand;

    switch (action)
    {
    case LightSwitchAction::Toggle:
        data->commandId = OnOff::Commands::Toggle::Id;

        break;

    case LightSwitchAction::On:
        data->commandId = OnOff::Commands::On::Id;
        break;

    case LightSwitchAction::Off:
        data->commandId = OnOff::Commands::Off::Id;
        break;

    default:
        Platform::Delete(data);
        return;
        break;
    }

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
}

void LightSwitchMgr::TriggerLevelControlAction(LevelControl::StepModeEnum stepMode, bool isGroupCommand)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();

    data->clusterId = chip::app::Clusters::LevelControl::Id;
    data->isGroup   = isGroupCommand;
    data->commandId = LevelControl::Commands::StepWithOnOff::Id;
    BindingCommandData::Step stepData{
        .stepMode = stepMode,
        .stepSize = LightSwitchMgr::stepCommand.stepSize,
        .transitionTime = LightSwitchMgr::stepCommand.transitionTime
    };
    stepData.optionsMask.Set(LightSwitchMgr::stepCommand.optionsMask);
    stepData.optionsOverride.Set(LightSwitchMgr::stepCommand.optionsOverride);
    data->commandData = stepData;
    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
}

void LightSwitchMgr::GenericSwitchWorkerFunction(intptr_t context)
{

    GenericSwitchEventData * data = reinterpret_cast<GenericSwitchEventData *>(context);

    switch (data->event)
    {
    case Switch::Events::InitialPress::Id: {
        uint8_t currentPosition = 1;

        // Set new attribute value
        Clusters::Switch::Attributes::CurrentPosition::Set(data->endpoint, currentPosition);

        // Trigger event
        Clusters::SwitchServer::Instance().OnInitialPress(data->endpoint, currentPosition);
        break;
    }
    case Switch::Events::ShortRelease::Id: {
        uint8_t previousPosition = 1;
        uint8_t currentPosition  = 0;

        // Set new attribute value
        Clusters::Switch::Attributes::CurrentPosition::Set(data->endpoint, currentPosition);

        // Trigger event
        Clusters::SwitchServer::Instance().OnShortRelease(data->endpoint, previousPosition);
        break;
    }
    default:
        break;
    }

    Platform::Delete(data);
}

void LightSwitchMgr::ButtonEventHandler(uint8_t button, uint8_t btnAction)
{
    AppEvent event = {};
    if (btnAction == to_underlying(SilabsPlatform::ButtonAction::ButtonPressed))
    {
        event = LightSwitchMgr::GetInstance().CreateNewEvent(button ? AppEvent::kEventType_ActionButtonPressed : AppEvent::kEventType_FunctionButtonPressed);
    }
    else
    {
        event = LightSwitchMgr::GetInstance().CreateNewEvent(button ? AppEvent::kEventType_ActionButtonReleased : AppEvent::kEventType_FunctionButtonReleased);
    }
    AppTask::GetAppTask().PostEvent(&event);
}

void LightSwitchMgr::AppEventHandler(AppEvent * aEvent)
{
    LightSwitchMgr * lightSwitch = static_cast<LightSwitchMgr *>(aEvent->LightSwitchEvent.Context);
    switch(aEvent->Type)
    {
    case AppEvent::kEventType_ResetWarning:
        lightSwitch->mResetWarning = true;
        AppTask::GetAppTask().StartFactoryResetSequence();
        break;
    case AppEvent::kEventType_ResetCanceled:
        lightSwitch->mResetWarning = false;
        AppTask::GetAppTask().CancelFactoryResetSequence();
        break;
    case AppEvent::kEventType_FunctionButtonPressed:
        lightSwitch->mFunctionButtonPressed = true;
        if (lightSwitch->mLongPressTimer)
        {
            lightSwitch->mLongPressTimer->Start();
        }
        if (lightSwitch->mActionButtonPressed)
        {
            lightSwitch->mActionButtonSuppressed = true;
            lightSwitch->stepDirection = (lightSwitch->stepDirection == StepModeEnum::kUp) ? StepModeEnum::kDown : StepModeEnum::kUp;
            ChipLogProgress(AppServer, "Step direction changed. Current Step Direction : %s", ((lightSwitch->stepDirection == StepModeEnum::kUp) ? "kUp" : "kDown"));
        }
        break;
    case AppEvent::kEventType_FunctionButtonReleased:
        lightSwitch->mFunctionButtonPressed = false;
        if (lightSwitch->mLongPressTimer)
        {
            lightSwitch->mLongPressTimer->Stop();
        }
        if (lightSwitch->mResetWarning)
        {
            aEvent->Type = AppEvent::kEventType_ResetCanceled;
            AppTask::GetAppTask().PostEvent(aEvent);
        }
        break;
    case AppEvent::kEventType_ActionButtonPressed:
        lightSwitch->mActionButtonPressed = true;
        aEvent->Handler = LightSwitchMgr::SwitchActionEventHandler;
        AppTask::GetAppTask().PostEvent(aEvent);
        if (lightSwitch->mLongPressTimer)
        {
            lightSwitch->mLongPressTimer->Start();
        }
        if (lightSwitch->mFunctionButtonPressed)
        {
            lightSwitch->mActionButtonSuppressed = true;
            lightSwitch->stepDirection = (lightSwitch->stepDirection == StepModeEnum::kUp) ? StepModeEnum::kDown : StepModeEnum::kUp;
            ChipLogProgress(AppServer, "Step direction changed. Current Step Direction : %s", ((lightSwitch->stepDirection == StepModeEnum::kUp) ? "kUp" : "kDown"));
        }
        break;
    case AppEvent::kEventType_ActionButtonReleased:
        lightSwitch->mActionButtonPressed = false;
        if (lightSwitch->mLongPressTimer)
        {
            lightSwitch->mLongPressTimer->Stop();
        }
        if (lightSwitch->mActionButtonSuppressed)
        {
            lightSwitch->mActionButtonSuppressed = false;
        }
        else
        {
            aEvent->Type = AppEvent::kEventType_TriggerToggle;
            aEvent->Handler = LightSwitchMgr::SwitchActionEventHandler;
            AppTask::GetAppTask().PostEvent(aEvent);
        }
        aEvent->Type = AppEvent::kEventType_ActionButtonReleased;
        aEvent->Handler = LightSwitchMgr::SwitchActionEventHandler;
        AppTask::GetAppTask().PostEvent(aEvent);
        break;
    case AppEvent::kEventType_TriggerLevelControlAction:
        aEvent->Handler = LightSwitchMgr::SwitchActionEventHandler;
        AppTask::GetAppTask().PostEvent(aEvent);
    default:
        break;
    }
}

void LightSwitchMgr::SwitchActionEventHandler(AppEvent * aEvent)
{
    switch(aEvent->Type)
    {
    case AppEvent::kEventType_ActionButtonPressed:
        LightSwitchMgr::GetInstance().GenericSwitchOnInitialPress();
        break;
    case AppEvent::kEventType_ActionButtonReleased:
        LightSwitchMgr::GetInstance().GenericSwitchOnShortRelease();
        break;
    case AppEvent::kEventType_TriggerLevelControlAction:
        LightSwitchMgr::GetInstance().TriggerLevelControlAction(LightSwitchMgr::GetInstance().getStepMode());
        break;
    case AppEvent::kEventType_TriggerToggle:
        LightSwitchMgr::GetInstance().TriggerLightSwitchAction(LightSwitchMgr::LightSwitchAction::Toggle);
        break;
    default:
        break;
    }
}
