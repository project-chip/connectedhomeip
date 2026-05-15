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

namespace {

using StepModeEnum = chip::app::Clusters::LevelControl::StepModeEnum;

chip::EndpointId gLightSwitchEndpoint   = chip::kInvalidEndpointId;
chip::EndpointId gGenericSwitchEndpoint = chip::kInvalidEndpointId;
StepModeEnum gStepDirection             = StepModeEnum::kUp;

bool sFunctionButtonPressed  = false;
bool sActionButtonPressed    = false;
bool sActionButtonSuppressed = false;
bool sIsButtonEventTriggered = false;

} // namespace

/**********************************************************
 * AppTask Definitions
 *********************************************************/

AppTask AppTask::sAppTask;

void AppTask::PostLevelControlActionEvent(void * context)
{
    AppTask * app = static_cast<AppTask *>(context);
    if (!app)
    {
        return;
    }
    AppEvent event;
    event.Handler = AppTask::AppEventHandler;
    if (sActionButtonPressed)
    {
        sActionButtonSuppressed = true;
        event.Type              = AppEvent::kEventType_TriggerLevelControlAction;
        app->PostEvent(&event);
    }
}

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

    longPressTimer = osTimerNew(PostLevelControlActionEvent, osTimerOnce, this, NULL);
    if (longPressTimer == NULL)
    {
        SILABS_LOG("Timer create failed");
        appError(APP_ERROR_CREATE_TIMER_FAILED);
    }

    return err;
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
    sAppTask.PostEvent(&event);
}

void AppTask::AppEventHandler(AppEvent * aEvent)
{
    switch (aEvent->Type)
    {
    case AppEvent::kEventType_FunctionButtonPressed:
        sFunctionButtonPressed = true;
        if (sActionButtonPressed)
        {
            sActionButtonSuppressed = true;
            gStepDirection = (gStepDirection == StepModeEnum::kUp) ? StepModeEnum::kDown : StepModeEnum::kUp;
            ChipLogProgress(AppServer, "Step direction changed. Current Step Direction : %s",
                            ((gStepDirection == StepModeEnum::kUp) ? "kUp" : "kDown"));
        }
        else
        {
            sIsButtonEventTriggered = true;
            // Post button press event to BaseApplication
            AppEvent button_event           = {};
            button_event.Type               = AppEvent::kEventType_Button;
            button_event.ButtonEvent.Action = static_cast<uint8_t>(SilabsPlatform::ButtonAction::ButtonPressed);
            button_event.Handler            = BaseApplication::ButtonHandler;
            sAppTask.PostEvent(&button_event);
        }
        break;

    case AppEvent::kEventType_FunctionButtonReleased:
        sFunctionButtonPressed = false;
        if (sIsButtonEventTriggered)
        {
            sIsButtonEventTriggered = false;
            // Post button release event to BaseApplication
            AppEvent button_event           = {};
            button_event.Type               = AppEvent::kEventType_Button;
            button_event.ButtonEvent.Action = static_cast<uint8_t>(SilabsPlatform::ButtonAction::ButtonReleased);
            button_event.Handler            = BaseApplication::ButtonHandler;
            sAppTask.PostEvent(&button_event);
        }
        break;

    case AppEvent::kEventType_ActionButtonPressed:
        sActionButtonPressed = true;
        {
            auto * switchData = Platform::New<GenericSwitchEventData>();
            switchData->endpoint = gGenericSwitchEndpoint;
            switchData->event    = Switch::Events::InitialPress::Id;
            TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().ScheduleWork(AppTask::GenericSwitchWorkerFunction,
                                                                             reinterpret_cast<intptr_t>(switchData));
        }
        if (sFunctionButtonPressed)
        {
            sActionButtonSuppressed = true;
            gStepDirection = (gStepDirection == StepModeEnum::kUp) ? StepModeEnum::kDown : StepModeEnum::kUp;
            ChipLogProgress(AppServer, "Step direction changed. Current Step Direction : %s",
                            ((gStepDirection == StepModeEnum::kUp) ? "kUp" : "kDown"));
        }
        else if (sAppTask.longPressTimer)
        {
            osStatus_t status = osTimerStart(sAppTask.longPressTimer, pdMS_TO_TICKS(LONG_PRESS_TIMEOUT_MS));
            if (status != osOK)
            {
                SILABS_LOG("Timer start() failed with error code : %lx", status);
                appError(APP_ERROR_START_TIMER_FAILED);
            }
        }
        break;

    case AppEvent::kEventType_ActionButtonReleased:
        sActionButtonPressed = false;
        if (sAppTask.longPressTimer)
        {
            if (osTimerStop(sAppTask.longPressTimer) == osError)
            {
                SILABS_LOG("Timer stop() failed");
                appError(APP_ERROR_STOP_TIMER_FAILED);
            }
        }
        if (sActionButtonSuppressed)
        {
            sActionButtonSuppressed = false;
        }
        else
        {
            BindingCommandData * toggleData = Platform::New<BindingCommandData>();
            toggleData->localEndpointId = gLightSwitchEndpoint;
            toggleData->clusterId       = Clusters::OnOff::Id;
            toggleData->isGroup         = false;
            toggleData->commandId       = OnOff::Commands::Toggle::Id;
            TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().ScheduleWork(AppTask::SwitchWorkerFunction,
                                                                             reinterpret_cast<intptr_t>(toggleData));
        }
        {
            auto * switchData = Platform::New<GenericSwitchEventData>();
            switchData->endpoint = gGenericSwitchEndpoint;
            switchData->event    = Switch::Events::ShortRelease::Id;
            TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().ScheduleWork(AppTask::GenericSwitchWorkerFunction,
                                                                             reinterpret_cast<intptr_t>(switchData));
        }
        break;

    case AppEvent::kEventType_TriggerLevelControlAction:
        {
            BindingCommandData * data = Platform::New<BindingCommandData>();
            data->localEndpointId = gLightSwitchEndpoint;
            data->clusterId       = Clusters::LevelControl::Id;
            data->isGroup         = false;
            data->commandId       = LevelControl::Commands::StepWithOnOff::Id;
            BindingCommandData::Step stepData{ .stepMode       = gStepDirection,
                                               .stepSize       = kStepCommand.stepSize,
                                               .transitionTime = kStepCommand.transitionTime };
            stepData.optionsMask.Set(kStepCommand.optionsMask);
            stepData.optionsOverride.Set(kStepCommand.optionsOverride);
            data->commandData = stepData;
            TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().ScheduleWork(AppTask::SwitchWorkerFunction,
                                                                             reinterpret_cast<intptr_t>(data));
        }
        break;

    default:
        break;
    }
}

CHIP_ERROR AppTask::InitLightSwitch(EndpointId lightSwitchEndpoint, EndpointId genericSwitchEndpoint)
{
    VerifyOrReturnError(lightSwitchEndpoint != kInvalidEndpointId, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(genericSwitchEndpoint != kInvalidEndpointId, CHIP_ERROR_INVALID_ARGUMENT);

    gLightSwitchEndpoint   = lightSwitchEndpoint;
    gGenericSwitchEndpoint = genericSwitchEndpoint;

    CHIP_ERROR scheduleErr = chip::DeviceLayer::PlatformMgr().ScheduleWork(&AppTask::InitBindingHandler, 0);
    if (scheduleErr != CHIP_NO_ERROR)
    {
        SILABS_LOG("InitBindingHandler() failed! Error: %" CHIP_ERROR_FORMAT, scheduleErr.Format());
        return scheduleErr;
    }

#if defined(ENABLE_CHIP_SHELL)
    LightSwitchCommands::RegisterSwitchCommands();
#endif // defined(ENABLE_CHIP_SHELL)

    return CHIP_NO_ERROR;
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

void AppTask::ProcessOnOffBindingCommand(CommandId commandId, const Binding::TableEntry & binding,
                                         OperationalDeviceProxy * peer_device)
{
    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        ChipLogProgress(NotSpecified, "OnOff command succeeds");
    };

    auto onFailure = [](CHIP_ERROR error) {
        ChipLogError(NotSpecified, "OnOff command failed: %" CHIP_ERROR_FORMAT, error.Format());
    };

    if (peer_device != nullptr)
    {
        VerifyOrDie(peer_device->ConnectionReady());
        Messaging::ExchangeManager * exchangeMgr = peer_device->GetExchangeManager();
        const SessionHandle & sessionHandle      = peer_device->GetSecureSession().Value();

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
        default:
            break;
        }
    }
    else
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
        default:
            break;
        }
    }
}

void AppTask::ProcessLevelControlBindingCommand(BindingCommandData * data, const Binding::TableEntry & binding,
                                                OperationalDeviceProxy * peer_device)
{
    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        ChipLogProgress(NotSpecified, "LevelControl command succeeds");
    };

    auto onFailure = [](CHIP_ERROR error) {
        ChipLogError(NotSpecified, "LevelControl command failed: %" CHIP_ERROR_FORMAT, error.Format());
    };

    if (peer_device != nullptr)
    {
        VerifyOrDie(peer_device->ConnectionReady());

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
    else
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
            ProcessOnOffBindingCommand(data->commandId, binding, nullptr);
            break;
        case Clusters::LevelControl::Id:
            ProcessLevelControlBindingCommand(data, binding, nullptr);
            break;
        }
    }
    else if (binding.type == Binding::MATTER_UNICAST_BINDING && !data->isGroup)
    {
        VerifyOrDie(peer_device != nullptr && peer_device->ConnectionReady());
        switch (data->clusterId)
        {
        case Clusters::OnOff::Id:
            ProcessOnOffBindingCommand(data->commandId, binding, peer_device);
            break;
        case Clusters::LevelControl::Id:
            ProcessLevelControlBindingCommand(data, binding, peer_device);
            break;
        }
    }
}

void AppTask::LightSwitchContextReleaseHandler(void * context)
{
    VerifyOrReturn(context != nullptr, ChipLogError(NotSpecified, "LightSwitchContextReleaseHandler: context is null"));
    Platform::Delete(static_cast<BindingCommandData *>(context));
}

void AppTask::InitBindingHandler(intptr_t arg)
{
    (void) arg;

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

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    AppTask::PostAttributeChangeCallback(attributePath, type, size, value);
}

void emberAfOnOffClusterInitCallback(EndpointId endpoint)
{
    (void) endpoint;
}
