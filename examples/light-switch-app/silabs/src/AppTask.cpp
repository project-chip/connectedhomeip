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
#include "LEDWidget.h"

#if defined(ENABLE_CHIP_SHELL)
#include "ShellCommands.h"
#endif // defined(ENABLE_CHIP_SHELL)

#ifdef DISPLAY_ENABLED
#include "lcd.h"
#ifdef QR_CODE_ENABLED
#include "qrcodegen.h"
#endif // QR_CODE_ENABLED
#endif // DISPLAY_ENABLED

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/CommandSender.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/bindings/BindingManager.h>
#include <app/clusters/switch-server/switch-server.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <assert.h>
#include <controller/InvokeInteraction.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
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

constexpr chip::app::Clusters::LevelControl::Commands::Step::Type kStepCommand = {
    .stepSize = 1, .transitionTime = 0, .optionsMask = 0, .optionsOverride = 0
};
} // namespace

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LevelControl;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Silabs;

using namespace chip::TLV;

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

    err = InitLightSwitch(kLightSwitchEndpoint, kGenericSwitchEndpoint);
    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("InitLightSwitch failed!");
        appError(err);
    }

    longPressTimer = new Timer(LONG_PRESS_TIMEOUT_MS, OnLongPressTimeout, this);

    return err;
}

void AppTask::Timer::Start()
{
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
            sAppTask.changeStepMode();
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
        sAppTask.SwitchActionEventHandler(aEvent->Type);
        if (functionButtonPressed)
        {
            actionButtonSuppressed = true;
            sAppTask.changeStepMode();
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
            sAppTask.SwitchActionEventHandler(aEvent->Type);
        }
        aEvent->Type = AppEvent::kEventType_ActionButtonReleased;
        sAppTask.SwitchActionEventHandler(aEvent->Type);
        break;
    case AppEvent::kEventType_TriggerLevelControlAction:
        sAppTask.SwitchActionEventHandler(aEvent->Type);
        break;
    default:
        break;
    }
}

CHIP_ERROR AppTask::InitLightSwitch(EndpointId lightSwitchEndpoint, EndpointId genericSwitchEndpoint)
{
    VerifyOrReturnError(lightSwitchEndpoint != kInvalidEndpointId, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(genericSwitchEndpoint != kInvalidEndpointId, CHIP_ERROR_INVALID_ARGUMENT);

    mLightSwitchEndpoint   = lightSwitchEndpoint;
    mGenericSwitchEndpoint = genericSwitchEndpoint;

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

void AppTask::GenericSwitchOnInitialPress()
{
    GenericSwitchEventData * data = Platform::New<GenericSwitchEventData>();

    data->endpoint = mGenericSwitchEndpoint;
    data->event    = Switch::Events::InitialPress::Id;

    TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().ScheduleWork(GenericSwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
}

void AppTask::GenericSwitchOnShortRelease()
{
    GenericSwitchEventData * data = Platform::New<GenericSwitchEventData>();

    data->endpoint = mGenericSwitchEndpoint;
    data->event    = Switch::Events::ShortRelease::Id;

    TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().ScheduleWork(GenericSwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
}

StepModeEnum AppTask::getStepMode()
{
    return mStepDirection;
}

void AppTask::changeStepMode()
{
    mStepDirection = (mStepDirection == StepModeEnum::kUp) ? StepModeEnum::kDown : StepModeEnum::kUp;
    ChipLogProgress(AppServer, "Step direction changed. Current Step Direction : %s",
                    ((getStepMode() == StepModeEnum::kUp) ? "kUp" : "kDown"));
}

void AppTask::TriggerLightSwitchAction(AppTask::LightSwitchAction action, bool isGroupCommand)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();

    data->clusterId = Clusters::OnOff::Id;
    data->isGroup   = isGroupCommand;

    switch (action)
    {
    case AppTask::LightSwitchAction::Toggle:
        data->commandId = OnOff::Commands::Toggle::Id;
        break;

    case AppTask::LightSwitchAction::On:
        data->commandId = OnOff::Commands::On::Id;
        break;

    case AppTask::LightSwitchAction::Off:
        data->commandId = OnOff::Commands::Off::Id;
        break;

    default:
        Platform::Delete(data);
        return;
    }

    TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
}

void AppTask::TriggerLevelControlAction(StepModeEnum stepMode, bool isGroupCommand)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();

    data->clusterId = Clusters::LevelControl::Id;
    data->isGroup   = isGroupCommand;
    data->commandId = LevelControl::Commands::StepWithOnOff::Id;

    BindingCommandData::Step stepData{ .stepMode       = stepMode,
                                       .stepSize       = kStepCommand.stepSize,
                                       .transitionTime = kStepCommand.transitionTime };
    stepData.optionsMask.Set(kStepCommand.optionsMask);
    stepData.optionsOverride.Set(kStepCommand.optionsOverride);
    data->commandData = stepData;

    TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
}

void AppTask::GenericSwitchWorkerFunction(intptr_t context)
{
    GenericSwitchEventData * data = reinterpret_cast<GenericSwitchEventData *>(context);

    switch (data->event)
    {
    case Switch::Events::InitialPress::Id: {
        uint8_t currentPosition = 1;

        auto switchCluster = Clusters::Switch::FindClusterOnEndpoint(data->endpoint);
        if (switchCluster != nullptr)
        {
            if (switchCluster->SetCurrentPosition(currentPosition) == CHIP_NO_ERROR)
            {
                RETURN_SAFELY_IGNORED switchCluster->OnInitialPress(currentPosition);
            }
        }
        break;
    }
    case Switch::Events::ShortRelease::Id: {
        uint8_t previousPosition = 1;
        uint8_t currentPosition  = 0;

        auto switchCluster = Clusters::Switch::FindClusterOnEndpoint(data->endpoint);
        if (switchCluster != nullptr)
        {
            if (switchCluster->SetCurrentPosition(currentPosition) == CHIP_NO_ERROR)
            {
                RETURN_SAFELY_IGNORED switchCluster->OnShortRelease(previousPosition);
            }
        }
        break;
    }
    default:
        break;
    }

    Platform::Delete(data);
}

void AppTask::SwitchActionEventHandler(uint16_t eventType)
{
    switch (eventType)
    {
    case AppEvent::kEventType_ActionButtonPressed:
        GenericSwitchOnInitialPress();
        break;
    case AppEvent::kEventType_ActionButtonReleased:
        GenericSwitchOnShortRelease();
        break;
    case AppEvent::kEventType_TriggerLevelControlAction:
        TriggerLevelControlAction(getStepMode());
        break;
    case AppEvent::kEventType_TriggerToggle:
        TriggerLightSwitchAction(AppTask::LightSwitchAction::Toggle);
        break;
    default:
        break;
    }
}

void AppTask::ProcessOnOffUnicastBindingCommand(CommandId commandId, const Binding::TableEntry & binding,
                                                Messaging::ExchangeManager * exchangeMgr, const SessionHandle & sessionHandle)
{
    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        ChipLogProgress(NotSpecified, "OnOff command succeeds");
    };

    auto onFailure = [](CHIP_ERROR error) {
        ChipLogError(NotSpecified, "OnOff command failed: %" CHIP_ERROR_FORMAT, error.Format());
    };

    switch (commandId)
    {
    case Clusters::OnOff::Commands::Toggle::Id: {
        Clusters::OnOff::Commands::Toggle::Type toggleCommand;
        TEMPORARY_RETURN_IGNORED Controller::InvokeCommandRequest(exchangeMgr, sessionHandle, binding.remote, toggleCommand,
                                                                  onSuccess, onFailure);
        break;
    }
    case Clusters::OnOff::Commands::On::Id: {
        Clusters::OnOff::Commands::On::Type onCommand;
        TEMPORARY_RETURN_IGNORED Controller::InvokeCommandRequest(exchangeMgr, sessionHandle, binding.remote, onCommand, onSuccess,
                                                                  onFailure);
        break;
    }
    case Clusters::OnOff::Commands::Off::Id: {
        Clusters::OnOff::Commands::Off::Type offCommand;
        TEMPORARY_RETURN_IGNORED Controller::InvokeCommandRequest(exchangeMgr, sessionHandle, binding.remote, offCommand, onSuccess,
                                                                  onFailure);
        break;
    }
    }
}

void AppTask::ProcessOnOffGroupBindingCommand(CommandId commandId, const Binding::TableEntry & binding)
{
    Messaging::ExchangeManager & exchangeMgr = Server::GetInstance().GetExchangeManager();

    switch (commandId)
    {
    case Clusters::OnOff::Commands::Toggle::Id: {
        Clusters::OnOff::Commands::Toggle::Type toggleCommand;
        TEMPORARY_RETURN_IGNORED Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId,
                                                                       toggleCommand);
        break;
    }
    case Clusters::OnOff::Commands::On::Id: {
        Clusters::OnOff::Commands::On::Type onCommand;
        TEMPORARY_RETURN_IGNORED Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId,
                                                                       onCommand);
        break;
    }
    case Clusters::OnOff::Commands::Off::Id: {
        Clusters::OnOff::Commands::Off::Type offCommand;
        TEMPORARY_RETURN_IGNORED Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId,
                                                                       offCommand);
        break;
    }
    }
}

void AppTask::ProcessLevelControlUnicastBindingCommand(BindingCommandData * data, const Binding::TableEntry & binding,
                                                       OperationalDeviceProxy * peer_device)
{
    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        ChipLogProgress(NotSpecified, "LevelControl command succeeds");
    };

    auto onFailure = [](CHIP_ERROR error) {
        ChipLogError(NotSpecified, "LevelControl command failed: %" CHIP_ERROR_FORMAT, error.Format());
    };

    VerifyOrDie(peer_device != nullptr && peer_device->ConnectionReady());

    switch (data->commandId)
    {
    case Clusters::LevelControl::Commands::MoveToLevel::Id: {
        Clusters::LevelControl::Commands::MoveToLevel::Type moveToLevelCommand;
        if (auto moveToLevel = std::get_if<BindingCommandData::MoveToLevel>(&data->commandData))
        {
            moveToLevelCommand.level           = moveToLevel->level;
            moveToLevelCommand.transitionTime  = moveToLevel->transitionTime;
            moveToLevelCommand.optionsMask     = moveToLevel->optionsMask;
            moveToLevelCommand.optionsOverride = moveToLevel->optionsOverride;
            TEMPORARY_RETURN_IGNORED Controller::InvokeCommandRequest(peer_device->GetExchangeManager(),
                                                                      peer_device->GetSecureSession().Value(), binding.remote,
                                                                      moveToLevelCommand, onSuccess, onFailure);
        }
        break;
    }
    case Clusters::LevelControl::Commands::Move::Id: {
        Clusters::LevelControl::Commands::Move::Type moveCommand;
        if (auto move = std::get_if<BindingCommandData::Move>(&data->commandData))
        {
            moveCommand.moveMode        = move->moveMode;
            moveCommand.rate            = move->rate;
            moveCommand.optionsMask     = move->optionsMask;
            moveCommand.optionsOverride = move->optionsOverride;
            TEMPORARY_RETURN_IGNORED Controller::InvokeCommandRequest(peer_device->GetExchangeManager(),
                                                                      peer_device->GetSecureSession().Value(), binding.remote,
                                                                      moveCommand, onSuccess, onFailure);
        }
        break;
    }
    case Clusters::LevelControl::Commands::Step::Id: {
        Clusters::LevelControl::Commands::Step::Type stepCommand;
        if (auto step = std::get_if<BindingCommandData::Step>(&data->commandData))
        {
            stepCommand.stepMode        = step->stepMode;
            stepCommand.stepSize        = step->stepSize;
            stepCommand.transitionTime  = step->transitionTime;
            stepCommand.optionsMask     = step->optionsMask;
            stepCommand.optionsOverride = step->optionsOverride;
            TEMPORARY_RETURN_IGNORED Controller::InvokeCommandRequest(peer_device->GetExchangeManager(),
                                                                      peer_device->GetSecureSession().Value(), binding.remote,
                                                                      stepCommand, onSuccess, onFailure);
        }
        break;
    }
    case Clusters::LevelControl::Commands::Stop::Id: {
        Clusters::LevelControl::Commands::Stop::Type stopCommand;
        if (auto stop = std::get_if<BindingCommandData::Stop>(&data->commandData))
        {
            stopCommand.optionsMask     = stop->optionsMask;
            stopCommand.optionsOverride = stop->optionsOverride;
            TEMPORARY_RETURN_IGNORED Controller::InvokeCommandRequest(peer_device->GetExchangeManager(),
                                                                      peer_device->GetSecureSession().Value(), binding.remote,
                                                                      stopCommand, onSuccess, onFailure);
        }
        break;
    }
    case Clusters::LevelControl::Commands::MoveToLevelWithOnOff::Id: {
        Clusters::LevelControl::Commands::MoveToLevelWithOnOff::Type moveToLevelWithOnOffCommand;
        if (auto moveToLevel = std::get_if<BindingCommandData::MoveToLevel>(&data->commandData))
        {
            moveToLevelWithOnOffCommand.level           = moveToLevel->level;
            moveToLevelWithOnOffCommand.transitionTime  = moveToLevel->transitionTime;
            moveToLevelWithOnOffCommand.optionsMask     = moveToLevel->optionsMask;
            moveToLevelWithOnOffCommand.optionsOverride = moveToLevel->optionsOverride;
            TEMPORARY_RETURN_IGNORED Controller::InvokeCommandRequest(peer_device->GetExchangeManager(),
                                                                      peer_device->GetSecureSession().Value(), binding.remote,
                                                                      moveToLevelWithOnOffCommand, onSuccess, onFailure);
        }
        break;
    }
    case Clusters::LevelControl::Commands::MoveWithOnOff::Id: {
        Clusters::LevelControl::Commands::MoveWithOnOff::Type moveWithOnOffCommand;
        if (auto move = std::get_if<BindingCommandData::Move>(&data->commandData))
        {
            moveWithOnOffCommand.moveMode        = move->moveMode;
            moveWithOnOffCommand.rate            = move->rate;
            moveWithOnOffCommand.optionsMask     = move->optionsMask;
            moveWithOnOffCommand.optionsOverride = move->optionsOverride;
            TEMPORARY_RETURN_IGNORED Controller::InvokeCommandRequest(peer_device->GetExchangeManager(),
                                                                      peer_device->GetSecureSession().Value(), binding.remote,
                                                                      moveWithOnOffCommand, onSuccess, onFailure);
        }
        break;
    }
    case Clusters::LevelControl::Commands::StepWithOnOff::Id: {
        Clusters::LevelControl::Commands::StepWithOnOff::Type stepWithOnOffCommand;
        if (auto step = std::get_if<BindingCommandData::Step>(&data->commandData))
        {
            stepWithOnOffCommand.stepMode        = step->stepMode;
            stepWithOnOffCommand.stepSize        = step->stepSize;
            stepWithOnOffCommand.transitionTime  = step->transitionTime;
            stepWithOnOffCommand.optionsMask     = step->optionsMask;
            stepWithOnOffCommand.optionsOverride = step->optionsOverride;
            TEMPORARY_RETURN_IGNORED Controller::InvokeCommandRequest(peer_device->GetExchangeManager(),
                                                                      peer_device->GetSecureSession().Value(), binding.remote,
                                                                      stepWithOnOffCommand, onSuccess, onFailure);
        }
        break;
    }
    case Clusters::LevelControl::Commands::StopWithOnOff::Id: {
        Clusters::LevelControl::Commands::StopWithOnOff::Type stopWithOnOffCommand;
        if (auto stop = std::get_if<BindingCommandData::Stop>(&data->commandData))
        {
            stopWithOnOffCommand.optionsMask     = stop->optionsMask;
            stopWithOnOffCommand.optionsOverride = stop->optionsOverride;
            TEMPORARY_RETURN_IGNORED Controller::InvokeCommandRequest(peer_device->GetExchangeManager(),
                                                                      peer_device->GetSecureSession().Value(), binding.remote,
                                                                      stopWithOnOffCommand, onSuccess, onFailure);
        }
        break;
    }
    default:
        break;
    }
}

void AppTask::ProcessLevelControlGroupBindingCommand(BindingCommandData * data, const Binding::TableEntry & binding)
{
    Messaging::ExchangeManager & exchangeMgr = Server::GetInstance().GetExchangeManager();

    switch (data->commandId)
    {
    case Clusters::LevelControl::Commands::MoveToLevel::Id: {
        Clusters::LevelControl::Commands::MoveToLevel::Type moveToLevelCommand;
        if (auto moveToLevel = std::get_if<BindingCommandData::MoveToLevel>(&data->commandData))
        {
            moveToLevelCommand.level           = moveToLevel->level;
            moveToLevelCommand.transitionTime  = moveToLevel->transitionTime;
            moveToLevelCommand.optionsMask     = moveToLevel->optionsMask;
            moveToLevelCommand.optionsOverride = moveToLevel->optionsOverride;
            TEMPORARY_RETURN_IGNORED Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId,
                                                                           moveToLevelCommand);
        }
        break;
    }
    case Clusters::LevelControl::Commands::Move::Id: {
        Clusters::LevelControl::Commands::Move::Type moveCommand;
        if (auto move = std::get_if<BindingCommandData::Move>(&data->commandData))
        {
            moveCommand.moveMode        = move->moveMode;
            moveCommand.rate            = move->rate;
            moveCommand.optionsMask     = move->optionsMask;
            moveCommand.optionsOverride = move->optionsOverride;
            TEMPORARY_RETURN_IGNORED Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId,
                                                                           moveCommand);
        }
        break;
    }
    case Clusters::LevelControl::Commands::Step::Id: {
        Clusters::LevelControl::Commands::Step::Type stepCommand;
        if (auto step = std::get_if<BindingCommandData::Step>(&data->commandData))
        {
            stepCommand.stepMode        = step->stepMode;
            stepCommand.stepSize        = step->stepSize;
            stepCommand.transitionTime  = step->transitionTime;
            stepCommand.optionsMask     = step->optionsMask;
            stepCommand.optionsOverride = step->optionsOverride;
            TEMPORARY_RETURN_IGNORED Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId,
                                                                           stepCommand);
        }
        break;
    }
    case Clusters::LevelControl::Commands::Stop::Id: {
        Clusters::LevelControl::Commands::Stop::Type stopCommand;
        if (auto stop = std::get_if<BindingCommandData::Stop>(&data->commandData))
        {
            stopCommand.optionsMask     = stop->optionsMask;
            stopCommand.optionsOverride = stop->optionsOverride;
            TEMPORARY_RETURN_IGNORED Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId,
                                                                           stopCommand);
        }
        break;
    }
    case Clusters::LevelControl::Commands::MoveToLevelWithOnOff::Id: {
        Clusters::LevelControl::Commands::MoveToLevelWithOnOff::Type moveToLevelWithOnOffCommand;
        if (auto moveToLevel = std::get_if<BindingCommandData::MoveToLevel>(&data->commandData))
        {
            moveToLevelWithOnOffCommand.level           = moveToLevel->level;
            moveToLevelWithOnOffCommand.transitionTime  = moveToLevel->transitionTime;
            moveToLevelWithOnOffCommand.optionsMask     = moveToLevel->optionsMask;
            moveToLevelWithOnOffCommand.optionsOverride = moveToLevel->optionsOverride;
            TEMPORARY_RETURN_IGNORED Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId,
                                                                           moveToLevelWithOnOffCommand);
        }
        break;
    }
    case Clusters::LevelControl::Commands::MoveWithOnOff::Id: {
        Clusters::LevelControl::Commands::MoveWithOnOff::Type moveWithOnOffCommand;
        if (auto move = std::get_if<BindingCommandData::Move>(&data->commandData))
        {
            moveWithOnOffCommand.moveMode        = move->moveMode;
            moveWithOnOffCommand.rate            = move->rate;
            moveWithOnOffCommand.optionsMask     = move->optionsMask;
            moveWithOnOffCommand.optionsOverride = move->optionsOverride;
            TEMPORARY_RETURN_IGNORED Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId,
                                                                           moveWithOnOffCommand);
        }
        break;
    }
    case Clusters::LevelControl::Commands::StepWithOnOff::Id: {
        Clusters::LevelControl::Commands::StepWithOnOff::Type stepWithOnOffCommand;
        if (auto step = std::get_if<BindingCommandData::Step>(&data->commandData))
        {
            stepWithOnOffCommand.stepMode        = step->stepMode;
            stepWithOnOffCommand.stepSize        = step->stepSize;
            stepWithOnOffCommand.transitionTime  = step->transitionTime;
            stepWithOnOffCommand.optionsMask     = step->optionsMask;
            stepWithOnOffCommand.optionsOverride = step->optionsOverride;
            TEMPORARY_RETURN_IGNORED Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId,
                                                                           stepWithOnOffCommand);
        }
        break;
    }
    case Clusters::LevelControl::Commands::StopWithOnOff::Id: {
        Clusters::LevelControl::Commands::StopWithOnOff::Type stopWithOnOffCommand;
        if (auto stop = std::get_if<BindingCommandData::Stop>(&data->commandData))
        {
            stopWithOnOffCommand.optionsMask     = stop->optionsMask;
            stopWithOnOffCommand.optionsOverride = stop->optionsOverride;
            TEMPORARY_RETURN_IGNORED Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId,
                                                                           stopWithOnOffCommand);
        }
        break;
    }
    default:
        break;
    }
}

void AppTask::LightSwitchChangedHandler(const Binding::TableEntry & binding, OperationalDeviceProxy * peer_device, void * context)
{
    VerifyOrReturn(context != nullptr, ChipLogError(NotSpecified, "OnDeviceConnectedFn: context is null"));
    BindingCommandData * data = static_cast<BindingCommandData *>(context);

    if (binding.type == Binding::MATTER_MULTICAST_BINDING && data->isGroup)
    {
        switch (data->clusterId)
        {
        case Clusters::OnOff::Id:
            ProcessOnOffGroupBindingCommand(data->commandId, binding);
            break;
        case Clusters::LevelControl::Id:
            ProcessLevelControlGroupBindingCommand(data, binding);
            break;
        }
    }
    else if (binding.type == Binding::MATTER_UNICAST_BINDING && !data->isGroup)
    {
        switch (data->clusterId)
        {
        case Clusters::OnOff::Id:
            VerifyOrDie(peer_device != nullptr && peer_device->ConnectionReady());
            ProcessOnOffUnicastBindingCommand(data->commandId, binding, peer_device->GetExchangeManager(),
                                              peer_device->GetSecureSession().Value());
            break;
        case Clusters::LevelControl::Id:
            ProcessLevelControlUnicastBindingCommand(data, binding, peer_device);
            break;
        }
    }
}

void AppTask::LightSwitchContextReleaseHandler(void * context)
{
    VerifyOrReturn(context != nullptr, ChipLogError(NotSpecified, "LightSwitchContextReleaseHandler: context is null"));
    Platform::Delete(static_cast<BindingCommandData *>(context));
}

void AppTask::InitBindingHandlerInternal(intptr_t arg)
{
    auto & server = chip::Server::GetInstance();
    LogErrorOnFailure(Binding::Manager::GetInstance().Init(
        { &server.GetFabricTable(), server.GetCASESessionManager(), &server.GetPersistentStorage() }));
    Binding::Manager::GetInstance().RegisterBoundDeviceChangedHandler(LightSwitchChangedHandler);
    Binding::Manager::GetInstance().RegisterBoundDeviceContextReleaseHandler(LightSwitchContextReleaseHandler);
}

void AppTask::SwitchWorkerFunction(intptr_t context)
{
    VerifyOrReturn(context != 0, ChipLogError(NotSpecified, "SwitchWorkerFunction - Invalid work data"));

    BindingCommandData * data = reinterpret_cast<BindingCommandData *>(context);
    CHIP_ERROR err            = Binding::Manager::GetInstance().NotifyBoundClusterChanged(data->localEndpointId, data->clusterId,
                                                                                          static_cast<void *>(data));

    if (err == CHIP_ERROR_INCORRECT_STATE || err == CHIP_ERROR_HANDLER_NOT_SET || err == CHIP_ERROR_NO_MEMORY)
    {
        Platform::Delete(data);
    }
}

void AppTask::BindingWorkerFunction(intptr_t context)
{
    VerifyOrReturn(context != 0, ChipLogError(NotSpecified, "BindingWorkerFunction - Invalid work data"));

    Binding::TableEntry * entry = reinterpret_cast<Binding::TableEntry *>(context);
    TEMPORARY_RETURN_IGNORED AddBindingEntry(*entry);

    Platform::Delete(entry);
}

CHIP_ERROR AppTask::InitBindingHandler()
{
    TEMPORARY_RETURN_IGNORED chip::DeviceLayer::PlatformMgr().ScheduleWork(InitBindingHandlerInternal);
    return CHIP_NO_ERROR;
}

void AppTask::PostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                          uint8_t * value)
{
    ClusterId clusterId                      = attributePath.mClusterId;
    [[maybe_unused]] AttributeId attributeId = attributePath.mAttributeId;
    ChipLogDetail(Zcl, "Cluster callback: " ChipLogFormatMEI, ChipLogValueMEI(clusterId));

    if (clusterId == Identify::Id)
    {
        ChipLogProgress(Zcl, "Identify attribute ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u",
                        ChipLogValueMEI(attributeId), type, *value, size);
    }
}

void AppTask::OnOffClusterInitCallback(chip::EndpointId endpoint)
{
    // TODO: implement any additional Cluster Server init actions
}

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    AppTask::PostAttributeChangeCallback(attributePath, type, size, value);
}

void emberAfOnOffClusterInitCallback(EndpointId endpoint)
{
    AppTask::OnOffClusterInitCallback(endpoint);
}
