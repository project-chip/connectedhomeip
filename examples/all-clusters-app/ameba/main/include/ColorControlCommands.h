/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "controller/InvokeInteraction.h"
#include "controller/ReadInteraction.h"
#include <app/clusters/binding-server/binding-cluster.h>

#if CONFIG_ENABLE_CHIP_SHELL
#include "lib/shell/Engine.h"
#include "lib/shell/commands/Help.h"
#endif // ENABLE_CHIP_SHELL

#if CONFIG_ENABLE_CHIP_SHELL
using chip::Shell::Engine;
using chip::Shell::shell_command_t;
using chip::Shell::streamer_get;
using chip::Shell::streamer_printf;

Engine sShellSwitchColorControlSubCommands;
Engine sShellSwitchColorControlReadSubCommands;
Engine sShellSwitchGroupsColorControlSubCommands;
#endif // defined(ENABLE_CHIP_SHELL)

using chip::OperationalDeviceProxy;
using chip::app::ConcreteCommandPath;
using chip::app::ConcreteDataAttributePath;
using chip::app::StatusIB;
using chip::app::Clusters::BindingTableEntry;

void ProcessColorControlUnicastBindingRead(BindingCommandData * data, const BindingTableEntry & binding,
                                           OperationalDeviceProxy * peer_device)
{
    auto onSuccess = [](const ConcreteDataAttributePath & attributePath, const auto & dataResponse) {
        ChipLogProgress(NotSpecified, "Read ColorControl attribute succeeds");
    };

    auto onFailure = [](const ConcreteDataAttributePath * attributePath, CHIP_ERROR error) {
        ChipLogError(NotSpecified, "Read ColorControl attribute failed: %" CHIP_ERROR_FORMAT, error.Format());
    };

    VerifyOrDie(peer_device != nullptr && peer_device->ConnectionReady());

    switch (data->attributeId)
    {
    case chip::app::Clusters::ColorControl::Attributes::AttributeList::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::AttributeList::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::CurrentHue::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::CurrentHue::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::CurrentSaturation::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::CurrentSaturation::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::RemainingTime::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::RemainingTime::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::CurrentX::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::CurrentX::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::CurrentY::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::CurrentY::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::DriftCompensation::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::DriftCompensation::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::CompensationText::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::CompensationText::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::ColorTemperatureMireds::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::ColorTemperatureMireds::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::ColorMode::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::ColorMode::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::Options::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::Options::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::NumberOfPrimaries::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::NumberOfPrimaries::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::Primary1X::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::Primary1X::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::Primary1Y::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::Primary1Y::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::Primary1Intensity::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::Primary1Intensity::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::Primary2X::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::Primary2X::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::Primary2Y::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::Primary2Y::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::Primary2Intensity::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::Primary2Intensity::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::Primary3X::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::Primary3X::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::Primary3Y::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::Primary3Y::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::Primary3Intensity::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::Primary3Intensity::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::Primary4X::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::Primary4X::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::Primary4Y::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::Primary4Y::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::Primary4Intensity::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::Primary4Intensity::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::Primary5X::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::Primary5X::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::Primary5Y::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::Primary5Y::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::Primary5Intensity::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::Primary5Intensity::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::Primary6X::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::Primary6X::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::Primary6Y::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::Primary6Y::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::Primary6Intensity::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::Primary6Intensity::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::WhitePointX::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::WhitePointX::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::WhitePointY::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::WhitePointY::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::ColorPointRX::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::ColorPointRX::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::ColorPointRY::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::ColorPointRY::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::ColorPointRIntensity::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::ColorPointRIntensity::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::ColorPointGX::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::ColorPointGX::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::ColorPointGY::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::ColorPointGY::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::ColorPointGIntensity::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::ColorPointGIntensity::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::ColorPointBX::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::ColorPointBX::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::ColorPointBY::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::ColorPointBY::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::ColorPointBIntensity::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::ColorPointBIntensity::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::EnhancedCurrentHue::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::EnhancedCurrentHue::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::EnhancedColorMode::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::EnhancedColorMode::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::ColorLoopActive::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::ColorLoopActive::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::ColorLoopDirection::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::ColorLoopDirection::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::ColorLoopTime::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::ColorLoopTime::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::ColorLoopStartEnhancedHue::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::ColorLoopStartEnhancedHue::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::ColorLoopStoredEnhancedHue::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::ColorLoopStoredEnhancedHue::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::ColorCapabilities::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::ColorCapabilities::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::ColorTempPhysicalMinMireds::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::ColorTempPhysicalMinMireds::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::ColorTempPhysicalMaxMireds::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::ColorTempPhysicalMaxMireds::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::CoupleColorTempToLevelMinMireds::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::CoupleColorTempToLevelMinMireds::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Attributes::StartUpColorTemperatureMireds::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::ColorControl::Attributes::StartUpColorTemperatureMireds::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;
    }
}

void ProcessColorControlUnicastBindingCommand(BindingCommandData * data, const BindingTableEntry & binding,
                                              OperationalDeviceProxy * peer_device)
{
    using namespace chip::app::Clusters::ColorControl;

    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        ChipLogProgress(NotSpecified, "ColorControl command succeeds");
    };

    auto onFailure = [](CHIP_ERROR error) {
        ChipLogError(NotSpecified, "ColorControl command failed: %" CHIP_ERROR_FORMAT, error.Format());
    };

    VerifyOrDie(peer_device != nullptr && peer_device->ConnectionReady());

    chip::app::Clusters::ColorControl::Commands::MoveToHue::Type moveToHueCommand;
    chip::app::Clusters::ColorControl::Commands::MoveHue::Type moveHueCommand;
    chip::app::Clusters::ColorControl::Commands::StepHue::Type stepHueCommand;
    chip::app::Clusters::ColorControl::Commands::MoveToSaturation::Type moveToSaturationCommand;
    chip::app::Clusters::ColorControl::Commands::MoveSaturation::Type moveSaturationCommand;
    chip::app::Clusters::ColorControl::Commands::StepSaturation::Type stepSaturationCommand;
    chip::app::Clusters::ColorControl::Commands::MoveToHueAndSaturation::Type moveToHueAndSaturationCommand;
    chip::app::Clusters::ColorControl::Commands::MoveToColor::Type moveToColorCommand;
    chip::app::Clusters::ColorControl::Commands::MoveColor::Type moveColorCommand;
    chip::app::Clusters::ColorControl::Commands::StepColor::Type stepColorCommand;
    chip::app::Clusters::ColorControl::Commands::MoveToColorTemperature::Type moveToColorTemperatureCommand;
    chip::app::Clusters::ColorControl::Commands::EnhancedMoveToHue::Type enhancedMoveToHueCommand;
    chip::app::Clusters::ColorControl::Commands::EnhancedMoveHue::Type enhancedMoveHueCommand;
    chip::app::Clusters::ColorControl::Commands::EnhancedStepHue::Type enhancedStepHueCommand;
    chip::app::Clusters::ColorControl::Commands::EnhancedMoveToHueAndSaturation::Type enhancedMoveToHueAndSaturationCommand;
    chip::app::Clusters::ColorControl::Commands::ColorLoopSet::Type colorLoopSetCommand;
    chip::app::Clusters::ColorControl::Commands::StopMoveStep::Type stopMoveStepCommand;
    chip::app::Clusters::ColorControl::Commands::MoveColorTemperature::Type moveColorTemperatureCommand;
    chip::app::Clusters::ColorControl::Commands::StepColorTemperature::Type stepColorTemperatureCommand;

    switch (data->commandId)
    {
    case chip::app::Clusters::ColorControl::Commands::MoveToHue::Id:
        moveToHueCommand.hue             = static_cast<uint8_t>(data->args[0]);
        moveToHueCommand.direction       = static_cast<DirectionEnum>(data->args[1]);
        moveToHueCommand.transitionTime  = static_cast<uint16_t>(data->args[2]);
        moveToHueCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[3]);
        moveToHueCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[4]);
        chip::Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(),
                                               binding.remote, moveToHueCommand, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Commands::MoveHue::Id:
        moveHueCommand.moveMode        = static_cast<MoveModeEnum>(data->args[0]);
        moveHueCommand.rate            = static_cast<uint8_t>(data->args[1]);
        moveHueCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[2]);
        moveHueCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[3]);
        chip::Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(),
                                               binding.remote, moveHueCommand, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Commands::StepHue::Id:
        stepHueCommand.stepMode        = static_cast<StepModeEnum>(data->args[0]);
        stepHueCommand.stepSize        = static_cast<uint8_t>(data->args[1]);
        stepHueCommand.transitionTime  = static_cast<uint8_t>(data->args[2]);
        stepHueCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[3]);
        stepHueCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[4]);
        chip::Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(),
                                               binding.remote, stepHueCommand, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Commands::MoveToSaturation::Id:
        moveToSaturationCommand.saturation      = static_cast<uint8_t>(data->args[0]);
        moveToSaturationCommand.transitionTime  = static_cast<uint16_t>(data->args[1]);
        moveToSaturationCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[2]);
        moveToSaturationCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[3]);
        chip::Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(),
                                               binding.remote, moveToSaturationCommand, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Commands::MoveSaturation::Id:
        moveSaturationCommand.moveMode        = static_cast<MoveModeEnum>(data->args[0]);
        moveSaturationCommand.rate            = static_cast<uint8_t>(data->args[1]);
        moveSaturationCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[2]);
        moveSaturationCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[3]);
        chip::Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(),
                                               binding.remote, moveSaturationCommand, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Commands::StepSaturation::Id:
        stepSaturationCommand.stepMode        = static_cast<StepModeEnum>(data->args[0]);
        stepSaturationCommand.stepSize        = static_cast<uint8_t>(data->args[1]);
        stepSaturationCommand.transitionTime  = static_cast<uint8_t>(data->args[2]);
        stepSaturationCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[3]);
        stepSaturationCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[4]);
        chip::Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(),
                                               binding.remote, stepSaturationCommand, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Commands::MoveToHueAndSaturation::Id:
        moveToHueAndSaturationCommand.hue             = static_cast<uint8_t>(data->args[0]);
        moveToHueAndSaturationCommand.saturation      = static_cast<uint8_t>(data->args[1]);
        moveToHueAndSaturationCommand.transitionTime  = static_cast<uint16_t>(data->args[2]);
        moveToHueAndSaturationCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[3]);
        moveToHueAndSaturationCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[4]);
        chip::Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(),
                                               binding.remote, moveToHueAndSaturationCommand, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Commands::MoveToColor::Id:
        moveToColorCommand.colorX          = static_cast<uint16_t>(data->args[0]);
        moveToColorCommand.colorY          = static_cast<uint16_t>(data->args[1]);
        moveToColorCommand.transitionTime  = static_cast<uint16_t>(data->args[2]);
        moveToColorCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[3]);
        moveToColorCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[4]);
        chip::Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(),
                                               binding.remote, moveToColorCommand, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Commands::MoveColor::Id:
        moveColorCommand.rateX           = static_cast<uint16_t>(data->args[0]);
        moveColorCommand.rateY           = static_cast<uint16_t>(data->args[1]);
        moveColorCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[2]);
        moveColorCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[3]);
        chip::Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(),
                                               binding.remote, moveColorCommand, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Commands::StepColor::Id:
        stepColorCommand.stepX           = static_cast<uint16_t>(data->args[0]);
        stepColorCommand.stepY           = static_cast<uint16_t>(data->args[1]);
        stepColorCommand.transitionTime  = static_cast<uint16_t>(data->args[2]);
        stepColorCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[3]);
        stepColorCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[4]);
        chip::Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(),
                                               binding.remote, stepColorCommand, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Commands::MoveToColorTemperature::Id:
        moveToColorTemperatureCommand.colorTemperatureMireds = static_cast<uint16_t>(data->args[0]);
        moveToColorTemperatureCommand.transitionTime         = static_cast<uint16_t>(data->args[1]);
        moveToColorTemperatureCommand.optionsMask            = static_cast<OptionsBitmap>(data->args[2]);
        moveToColorTemperatureCommand.optionsOverride        = static_cast<OptionsBitmap>(data->args[3]);
        chip::Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(),
                                               binding.remote, moveToColorTemperatureCommand, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Commands::EnhancedMoveToHue::Id:
        enhancedMoveToHueCommand.enhancedHue     = static_cast<uint16_t>(data->args[0]);
        enhancedMoveToHueCommand.direction       = static_cast<DirectionEnum>(data->args[1]);
        enhancedMoveToHueCommand.transitionTime  = static_cast<uint16_t>(data->args[2]);
        enhancedMoveToHueCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[3]);
        enhancedMoveToHueCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[4]);
        chip::Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(),
                                               binding.remote, enhancedMoveToHueCommand, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Commands::EnhancedMoveHue::Id:
        enhancedMoveHueCommand.moveMode        = static_cast<MoveModeEnum>(data->args[0]);
        enhancedMoveHueCommand.rate            = static_cast<uint16_t>(data->args[1]);
        enhancedMoveHueCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[2]);
        enhancedMoveHueCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[3]);
        chip::Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(),
                                               binding.remote, enhancedMoveHueCommand, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Commands::EnhancedStepHue::Id:
        enhancedStepHueCommand.stepMode        = static_cast<StepModeEnum>(data->args[0]);
        enhancedStepHueCommand.stepSize        = static_cast<uint16_t>(data->args[1]);
        enhancedStepHueCommand.transitionTime  = static_cast<uint16_t>(data->args[2]);
        enhancedStepHueCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[3]);
        enhancedStepHueCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[4]);
        chip::Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(),
                                               binding.remote, enhancedStepHueCommand, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Commands::EnhancedMoveToHueAndSaturation::Id:
        enhancedMoveToHueAndSaturationCommand.enhancedHue     = static_cast<uint16_t>(data->args[0]);
        enhancedMoveToHueAndSaturationCommand.saturation      = static_cast<uint8_t>(data->args[1]);
        enhancedMoveToHueAndSaturationCommand.transitionTime  = static_cast<uint16_t>(data->args[2]);
        enhancedMoveToHueAndSaturationCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[3]);
        enhancedMoveToHueAndSaturationCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[4]);
        chip::Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(),
                                               binding.remote, enhancedMoveToHueAndSaturationCommand, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Commands::ColorLoopSet::Id:
        colorLoopSetCommand.updateFlags =
            static_cast<chip::BitMask<chip::app::Clusters::ColorControl::UpdateFlagsBitmap>>(data->args[0]);
        colorLoopSetCommand.action          = static_cast<ColorLoopActionEnum>(data->args[1]);
        colorLoopSetCommand.direction       = static_cast<ColorLoopDirectionEnum>(data->args[2]);
        colorLoopSetCommand.time            = static_cast<uint16_t>(data->args[3]);
        colorLoopSetCommand.startHue        = static_cast<uint16_t>(data->args[4]);
        colorLoopSetCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[5]);
        colorLoopSetCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[6]);
        chip::Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(),
                                               binding.remote, colorLoopSetCommand, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Commands::StopMoveStep::Id:
        stopMoveStepCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[0]);
        stopMoveStepCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[1]);
        chip::Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(),
                                               binding.remote, stopMoveStepCommand, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Commands::MoveColorTemperature::Id:
        moveColorTemperatureCommand.moveMode                      = static_cast<MoveModeEnum>(data->args[0]);
        moveColorTemperatureCommand.rate                          = static_cast<uint16_t>(data->args[1]);
        moveColorTemperatureCommand.colorTemperatureMinimumMireds = static_cast<uint16_t>(data->args[2]);
        moveColorTemperatureCommand.colorTemperatureMaximumMireds = static_cast<uint16_t>(data->args[3]);
        moveColorTemperatureCommand.optionsMask                   = static_cast<OptionsBitmap>(data->args[4]);
        moveColorTemperatureCommand.optionsOverride               = static_cast<OptionsBitmap>(data->args[5]);
        chip::Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(),
                                               binding.remote, moveColorTemperatureCommand, onSuccess, onFailure);
        break;

    case chip::app::Clusters::ColorControl::Commands::StepColorTemperature::Id:
        stepColorTemperatureCommand.stepMode                      = static_cast<StepModeEnum>(data->args[0]);
        stepColorTemperatureCommand.stepSize                      = static_cast<uint16_t>(data->args[1]);
        stepColorTemperatureCommand.transitionTime                = static_cast<uint16_t>(data->args[2]);
        stepColorTemperatureCommand.colorTemperatureMinimumMireds = static_cast<uint16_t>(data->args[3]);
        stepColorTemperatureCommand.colorTemperatureMaximumMireds = static_cast<uint16_t>(data->args[4]);
        stepColorTemperatureCommand.optionsMask                   = static_cast<OptionsBitmap>(data->args[5]);
        stepColorTemperatureCommand.optionsOverride               = static_cast<OptionsBitmap>(data->args[6]);
        chip::Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(),
                                               binding.remote, stepColorTemperatureCommand, onSuccess, onFailure);
        break;
    }
}

void ProcessColorControlGroupBindingCommand(BindingCommandData * data, const BindingTableEntry & binding)
{
    using namespace chip::app::Clusters::ColorControl;

    chip::Messaging::ExchangeManager & exchangeMgr = chip::Server::GetInstance().GetExchangeManager();

    chip::app::Clusters::ColorControl::Commands::MoveToHue::Type moveToHueCommand;
    chip::app::Clusters::ColorControl::Commands::MoveHue::Type moveHueCommand;
    chip::app::Clusters::ColorControl::Commands::StepHue::Type stepHueCommand;
    chip::app::Clusters::ColorControl::Commands::MoveToSaturation::Type moveToSaturationCommand;
    chip::app::Clusters::ColorControl::Commands::MoveSaturation::Type moveSaturationCommand;
    chip::app::Clusters::ColorControl::Commands::StepSaturation::Type stepSaturationCommand;
    chip::app::Clusters::ColorControl::Commands::MoveToHueAndSaturation::Type moveToHueAndSaturationCommand;
    chip::app::Clusters::ColorControl::Commands::MoveToColor::Type moveToColorCommand;
    chip::app::Clusters::ColorControl::Commands::MoveColor::Type moveColorCommand;
    chip::app::Clusters::ColorControl::Commands::StepColor::Type stepColorCommand;
    chip::app::Clusters::ColorControl::Commands::MoveToColorTemperature::Type moveToColorTemperatureCommand;
    chip::app::Clusters::ColorControl::Commands::EnhancedMoveToHue::Type enhancedMoveToHueCommand;
    chip::app::Clusters::ColorControl::Commands::EnhancedMoveHue::Type enhancedMoveHueCommand;
    chip::app::Clusters::ColorControl::Commands::EnhancedStepHue::Type enhancedStepHueCommand;
    chip::app::Clusters::ColorControl::Commands::EnhancedMoveToHueAndSaturation::Type enhancedMoveToHueAndSaturationCommand;
    chip::app::Clusters::ColorControl::Commands::ColorLoopSet::Type colorLoopSetCommand;
    chip::app::Clusters::ColorControl::Commands::StopMoveStep::Type stopMoveStepCommand;
    chip::app::Clusters::ColorControl::Commands::MoveColorTemperature::Type moveColorTemperatureCommand;
    chip::app::Clusters::ColorControl::Commands::StepColorTemperature::Type stepColorTemperatureCommand;

    switch (data->commandId)
    {
    case chip::app::Clusters::ColorControl::Commands::MoveToHue::Id:
        moveToHueCommand.hue             = static_cast<uint8_t>(data->args[0]);
        moveToHueCommand.direction       = static_cast<DirectionEnum>(data->args[1]);
        moveToHueCommand.transitionTime  = static_cast<uint16_t>(data->args[2]);
        moveToHueCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[3]);
        moveToHueCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[4]);
        chip::Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, moveToHueCommand);
        break;

    case chip::app::Clusters::ColorControl::Commands::MoveHue::Id:
        moveHueCommand.moveMode        = static_cast<MoveModeEnum>(data->args[0]);
        moveHueCommand.rate            = static_cast<uint8_t>(data->args[1]);
        moveHueCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[2]);
        moveHueCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[3]);
        chip::Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, moveHueCommand);
        break;

    case chip::app::Clusters::ColorControl::Commands::StepHue::Id:
        stepHueCommand.stepMode        = static_cast<StepModeEnum>(data->args[0]);
        stepHueCommand.stepSize        = static_cast<uint8_t>(data->args[1]);
        stepHueCommand.transitionTime  = static_cast<uint8_t>(data->args[2]);
        stepHueCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[3]);
        stepHueCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[4]);
        chip::Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, stepHueCommand);
        break;

    case chip::app::Clusters::ColorControl::Commands::MoveToSaturation::Id:
        moveToSaturationCommand.saturation      = static_cast<uint8_t>(data->args[0]);
        moveToSaturationCommand.transitionTime  = static_cast<uint16_t>(data->args[1]);
        moveToSaturationCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[2]);
        moveToSaturationCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[3]);
        chip::Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, moveToSaturationCommand);
        break;

    case chip::app::Clusters::ColorControl::Commands::MoveSaturation::Id:
        moveSaturationCommand.moveMode        = static_cast<MoveModeEnum>(data->args[0]);
        moveSaturationCommand.rate            = static_cast<uint8_t>(data->args[1]);
        moveSaturationCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[2]);
        moveSaturationCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[3]);
        chip::Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, moveSaturationCommand);
        break;

    case chip::app::Clusters::ColorControl::Commands::StepSaturation::Id:
        stepSaturationCommand.stepMode        = static_cast<StepModeEnum>(data->args[0]);
        stepSaturationCommand.stepSize        = static_cast<uint8_t>(data->args[1]);
        stepSaturationCommand.transitionTime  = static_cast<uint8_t>(data->args[2]);
        stepSaturationCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[3]);
        stepSaturationCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[4]);
        chip::Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, stepSaturationCommand);
        break;

    case chip::app::Clusters::ColorControl::Commands::MoveToHueAndSaturation::Id:
        moveToHueAndSaturationCommand.hue             = static_cast<uint8_t>(data->args[0]);
        moveToHueAndSaturationCommand.saturation      = static_cast<uint8_t>(data->args[1]);
        moveToHueAndSaturationCommand.transitionTime  = static_cast<uint16_t>(data->args[2]);
        moveToHueAndSaturationCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[3]);
        moveToHueAndSaturationCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[4]);
        chip::Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId,
                                                    moveToHueAndSaturationCommand);
        break;

    case chip::app::Clusters::ColorControl::Commands::MoveToColor::Id:
        moveToColorCommand.colorX          = static_cast<uint16_t>(data->args[0]);
        moveToColorCommand.colorY          = static_cast<uint16_t>(data->args[1]);
        moveToColorCommand.transitionTime  = static_cast<uint16_t>(data->args[2]);
        moveToColorCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[3]);
        moveToColorCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[4]);
        chip::Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, moveToColorCommand);
        break;

    case chip::app::Clusters::ColorControl::Commands::MoveColor::Id:
        moveColorCommand.rateX           = static_cast<uint16_t>(data->args[0]);
        moveColorCommand.rateY           = static_cast<uint16_t>(data->args[1]);
        moveColorCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[2]);
        moveColorCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[3]);
        chip::Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, moveColorCommand);
        break;

    case chip::app::Clusters::ColorControl::Commands::StepColor::Id:
        stepColorCommand.stepX           = static_cast<uint16_t>(data->args[0]);
        stepColorCommand.stepY           = static_cast<uint16_t>(data->args[1]);
        stepColorCommand.transitionTime  = static_cast<uint16_t>(data->args[2]);
        stepColorCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[3]);
        stepColorCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[4]);
        chip::Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, stepColorCommand);
        break;

    case chip::app::Clusters::ColorControl::Commands::MoveToColorTemperature::Id:
        moveToColorTemperatureCommand.colorTemperatureMireds = static_cast<uint16_t>(data->args[0]);
        moveToColorTemperatureCommand.transitionTime         = static_cast<uint16_t>(data->args[1]);
        moveToColorTemperatureCommand.optionsMask            = static_cast<OptionsBitmap>(data->args[2]);
        moveToColorTemperatureCommand.optionsOverride        = static_cast<OptionsBitmap>(data->args[3]);
        chip::Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId,
                                                    moveToColorTemperatureCommand);
        break;

    case chip::app::Clusters::ColorControl::Commands::EnhancedMoveToHue::Id:
        enhancedMoveToHueCommand.enhancedHue     = static_cast<uint16_t>(data->args[0]);
        enhancedMoveToHueCommand.direction       = static_cast<DirectionEnum>(data->args[1]);
        enhancedMoveToHueCommand.transitionTime  = static_cast<uint16_t>(data->args[2]);
        enhancedMoveToHueCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[3]);
        enhancedMoveToHueCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[4]);
        chip::Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, enhancedMoveToHueCommand);
        break;

    case chip::app::Clusters::ColorControl::Commands::EnhancedMoveHue::Id:
        enhancedMoveHueCommand.moveMode        = static_cast<MoveModeEnum>(data->args[0]);
        enhancedMoveHueCommand.rate            = static_cast<uint16_t>(data->args[1]);
        enhancedMoveHueCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[2]);
        enhancedMoveHueCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[3]);
        chip::Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, enhancedMoveHueCommand);
        break;

    case chip::app::Clusters::ColorControl::Commands::EnhancedStepHue::Id:
        enhancedStepHueCommand.stepMode        = static_cast<StepModeEnum>(data->args[0]);
        enhancedStepHueCommand.stepSize        = static_cast<uint16_t>(data->args[1]);
        enhancedStepHueCommand.transitionTime  = static_cast<uint16_t>(data->args[2]);
        enhancedStepHueCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[3]);
        enhancedStepHueCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[4]);
        chip::Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, enhancedStepHueCommand);
        break;

    case chip::app::Clusters::ColorControl::Commands::EnhancedMoveToHueAndSaturation::Id:
        enhancedMoveToHueAndSaturationCommand.enhancedHue     = static_cast<uint16_t>(data->args[0]);
        enhancedMoveToHueAndSaturationCommand.saturation      = static_cast<uint8_t>(data->args[1]);
        enhancedMoveToHueAndSaturationCommand.transitionTime  = static_cast<uint16_t>(data->args[2]);
        enhancedMoveToHueAndSaturationCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[3]);
        enhancedMoveToHueAndSaturationCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[4]);
        chip::Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId,
                                                    enhancedMoveToHueAndSaturationCommand);
        break;

    case chip::app::Clusters::ColorControl::Commands::ColorLoopSet::Id:
        colorLoopSetCommand.updateFlags =
            static_cast<chip::BitMask<chip::app::Clusters::ColorControl::UpdateFlagsBitmap>>(data->args[0]);
        colorLoopSetCommand.action          = static_cast<ColorLoopActionEnum>(data->args[1]);
        colorLoopSetCommand.direction       = static_cast<ColorLoopDirectionEnum>(data->args[2]);
        colorLoopSetCommand.time            = static_cast<uint16_t>(data->args[3]);
        colorLoopSetCommand.startHue        = static_cast<uint16_t>(data->args[4]);
        colorLoopSetCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[5]);
        colorLoopSetCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[6]);
        chip::Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, colorLoopSetCommand);
        break;

    case chip::app::Clusters::ColorControl::Commands::StopMoveStep::Id:
        stopMoveStepCommand.optionsMask     = static_cast<OptionsBitmap>(data->args[0]);
        stopMoveStepCommand.optionsOverride = static_cast<OptionsBitmap>(data->args[1]);
        chip::Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, stopMoveStepCommand);
        break;

    case chip::app::Clusters::ColorControl::Commands::MoveColorTemperature::Id:
        moveColorTemperatureCommand.moveMode                      = static_cast<MoveModeEnum>(data->args[0]);
        moveColorTemperatureCommand.rate                          = static_cast<uint16_t>(data->args[1]);
        moveColorTemperatureCommand.colorTemperatureMinimumMireds = static_cast<uint16_t>(data->args[2]);
        moveColorTemperatureCommand.colorTemperatureMaximumMireds = static_cast<uint16_t>(data->args[3]);
        moveColorTemperatureCommand.optionsMask                   = static_cast<OptionsBitmap>(data->args[4]);
        moveColorTemperatureCommand.optionsOverride               = static_cast<OptionsBitmap>(data->args[5]);
        chip::Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId,
                                                    moveColorTemperatureCommand);
        break;

    case chip::app::Clusters::ColorControl::Commands::StepColorTemperature::Id:
        stepColorTemperatureCommand.stepMode                      = static_cast<StepModeEnum>(data->args[0]);
        stepColorTemperatureCommand.stepSize                      = static_cast<uint16_t>(data->args[1]);
        stepColorTemperatureCommand.transitionTime                = static_cast<uint16_t>(data->args[2]);
        stepColorTemperatureCommand.colorTemperatureMinimumMireds = static_cast<uint16_t>(data->args[3]);
        stepColorTemperatureCommand.colorTemperatureMaximumMireds = static_cast<uint16_t>(data->args[4]);
        stepColorTemperatureCommand.optionsMask                   = static_cast<OptionsBitmap>(data->args[5]);
        stepColorTemperatureCommand.optionsOverride               = static_cast<OptionsBitmap>(data->args[6]);
        chip::Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId,
                                                    stepColorTemperatureCommand);
        break;
    }
}

#if CONFIG_ENABLE_CHIP_SHELL
/********************************************************
 * ColorControl switch shell functions
 *********************************************************/

CHIP_ERROR ColorControlHelpHandler(int argc, char ** argv)
{
    sShellSwitchColorControlSubCommands.ForEachCommand(chip::Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlSwitchCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return ColorControlHelpHandler(argc, argv);
    }

    return sShellSwitchColorControlSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR MoveToHueCommandHandler(int argc, char ** argv)
{
    if (argc != 5)
    {
        return ColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::MoveToHue::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->args[4]             = atoi(argv[4]);

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR MoveHueCommandHandler(int argc, char ** argv)
{
    if (argc != 4)
    {
        return ColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::MoveHue::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR StepHueCommandHandler(int argc, char ** argv)
{
    if (argc != 5)
    {
        return ColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::StepHue::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->args[4]             = atoi(argv[4]);

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR MoveToSaturationCommandHandler(int argc, char ** argv)
{
    if (argc != 4)
    {
        return ColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::MoveToSaturation::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR MoveSaturationCommandHandler(int argc, char ** argv)
{
    if (argc != 4)
    {
        return ColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::MoveSaturation::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR StepSaturationCommandHandler(int argc, char ** argv)
{
    if (argc != 5)
    {
        return ColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::StepSaturation::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->args[4]             = atoi(argv[4]);

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR MoveToHueAndSaturationCommandHandler(int argc, char ** argv)
{
    if (argc != 5)
    {
        return ColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::MoveToHueAndSaturation::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->args[4]             = atoi(argv[4]);

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR MoveToColorCommandHandler(int argc, char ** argv)
{
    if (argc != 5)
    {
        return ColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::MoveToColor::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->args[4]             = atoi(argv[4]);

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR MoveColorCommandHandler(int argc, char ** argv)
{
    if (argc != 4)
    {
        return ColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::MoveColor::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR StepColorCommandHandler(int argc, char ** argv)
{
    if (argc != 5)
    {
        return ColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::StepColor::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->args[4]             = atoi(argv[4]);

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR MoveToColorTemperatureCommandHandler(int argc, char ** argv)
{
    if (argc != 4)
    {
        return ColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::MoveToColorTemperature::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR EnhancedMoveToHueCommandHandler(int argc, char ** argv)
{
    if (argc != 5)
    {
        return ColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::EnhancedMoveToHue::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->args[4]             = atoi(argv[4]);

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR EnhancedMoveHueCommandHandler(int argc, char ** argv)
{
    if (argc != 4)
    {
        return ColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::EnhancedMoveHue::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR EnhancedStepHueCommandHandler(int argc, char ** argv)
{
    if (argc != 5)
    {
        return ColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::EnhancedStepHue::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->args[4]             = atoi(argv[4]);

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR EnhancedMoveToHueAndSaturationCommandHandler(int argc, char ** argv)
{
    if (argc != 5)
    {
        return ColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::EnhancedMoveToHueAndSaturation::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->args[4]             = atoi(argv[4]);

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorLoopSetCommandHandler(int argc, char ** argv)
{
    if (argc != 7)
    {
        return ColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::ColorLoopSet::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->args[4]             = atoi(argv[4]);
    data->args[5]             = atoi(argv[5]);
    data->args[6]             = atoi(argv[6]);

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR StopMoveStepCommandHandler(int argc, char ** argv)
{
    if (argc != 2)
    {
        return ColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::StopMoveStep::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR MoveColorTemperatureCommandHandler(int argc, char ** argv)
{
    if (argc != 6)
    {
        return ColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::MoveColorTemperature::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->args[4]             = atoi(argv[4]);
    data->args[5]             = atoi(argv[5]);

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR StepColorTemperatureCommandHandler(int argc, char ** argv)
{
    if (argc != 7)
    {
        return ColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::StepColorTemperature::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->args[4]             = atoi(argv[4]);
    data->args[5]             = atoi(argv[5]);
    data->args[6]             = atoi(argv[6]);

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

/********************************************************
 * ColorControl Read switch shell functions
 *********************************************************/

CHIP_ERROR ColorControlReadHelpHandler(int argc, char ** argv)
{
    sShellSwitchColorControlReadSubCommands.ForEachCommand(chip::Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlRead(int argc, char ** argv)
{
    if (argc == 0)
    {
        return ColorControlReadHelpHandler(argc, argv);
    }

    return sShellSwitchColorControlReadSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR ColorControlReadAttributeList(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::AttributeList::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadCurrentHue(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::CurrentHue::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadCurrentSaturation(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::CurrentSaturation::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadRemainingTime(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::RemainingTime::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadCurrentX(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::CurrentX::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadCurrentY(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::CurrentY::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadDriftCompensation(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::DriftCompensation::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadCompensationText(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::CompensationText::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadColorTemperature(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::ColorTemperatureMireds::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadColorMode(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::ColorMode::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadOptions(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::Options::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadNumberOfPrimaries(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::NumberOfPrimaries::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadPrimary1X(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::Primary1X::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadPrimary1Y(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::Primary1Y::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadPrimary1Intensity(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::Primary1Intensity::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadPrimary2X(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::Primary2X::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadPrimary2Y(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::Primary2Y::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadPrimary2Intensity(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::Primary2Intensity::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadPrimary3X(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::Primary3X::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadPrimary3Y(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::Primary3Y::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadPrimary3Intensity(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::Primary3Intensity::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadPrimary4X(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::Primary4X::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadPrimary4Y(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::Primary4Y::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadPrimary4Intensity(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::Primary4Intensity::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadPrimary5X(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::Primary5X::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadPrimary5Y(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::Primary5Y::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadPrimary5Intensity(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::Primary5Intensity::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadPrimary6X(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::Primary6X::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadPrimary6Y(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::Primary6Y::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadPrimary6Intensity(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::Primary6Intensity::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadWhitePointX(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::WhitePointX::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadWhitePointY(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::WhitePointY::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadColorPointRX(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::ColorPointRX::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadColorPointRY(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::ColorPointRY::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadColorPointRIntensity(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::ColorPointRIntensity::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadColorPointGX(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::ColorPointGX::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadColorPointGY(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::ColorPointGY::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadColorPointGIntensity(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::ColorPointGIntensity::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadColorPointBX(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::ColorPointBX::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadColorPointBY(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::ColorPointBY::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadColorPointBIntensity(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::ColorPointBIntensity::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadEnhancedCurrentHue(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::EnhancedCurrentHue::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadEnhancedColorMode(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::EnhancedColorMode::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadColorLoopActive(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::ColorLoopActive::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadColorLoopDirection(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::ColorLoopDirection::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadColorLoopTime(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::ColorLoopTime::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadColorLoopStartEnhancedHue(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::ColorLoopStartEnhancedHue::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadColorLoopStoredEnhancedHue(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::ColorLoopStoredEnhancedHue::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadColorCapabilities(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::ColorCapabilities::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadColorTempPhysicalMinMireds(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::ColorTempPhysicalMinMireds::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadColorTempPhysicalMaxMireds(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::ColorTempPhysicalMaxMireds::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadCoupleColorTempToLevelMinMireds(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::CoupleColorTempToLevelMinMireds::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlReadStartUpColorTemperatureMireds(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::ColorControl::Attributes::StartUpColorTemperatureMireds::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

/********************************************************
 * Groups ColorControl switch shell functions
 *********************************************************/

CHIP_ERROR GroupsColorControlHelpHandler(int argc, char ** argv)
{
    sShellSwitchGroupsColorControlSubCommands.ForEachCommand(chip::Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsColorControlSwitchCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return GroupsColorControlHelpHandler(argc, argv);
    }

    return sShellSwitchGroupsColorControlSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR GroupsMoveToHueCommandHandler(int argc, char ** argv)
{
    if (argc != 5)
    {
        return GroupsColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::MoveToHue::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->args[4]             = atoi(argv[4]);
    data->isGroup             = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsMoveHueCommandHandler(int argc, char ** argv)
{
    if (argc != 4)
    {
        return GroupsColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::MoveHue::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->isGroup             = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsStepHueCommandHandler(int argc, char ** argv)
{
    if (argc != 5)
    {
        return GroupsColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::StepHue::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->args[4]             = atoi(argv[4]);
    data->isGroup             = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsMoveToSaturationCommandHandler(int argc, char ** argv)
{
    if (argc != 4)
    {
        return GroupsColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::MoveToSaturation::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->isGroup             = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsMoveSaturationCommandHandler(int argc, char ** argv)
{
    if (argc != 4)
    {
        return GroupsColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::MoveSaturation::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->isGroup             = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsStepSaturationCommandHandler(int argc, char ** argv)
{
    if (argc != 5)
    {
        return GroupsColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::StepSaturation::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->args[4]             = atoi(argv[4]);
    data->isGroup             = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsMoveToHueAndSaturationCommandHandler(int argc, char ** argv)
{
    if (argc != 5)
    {
        return GroupsColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::MoveToHueAndSaturation::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->args[4]             = atoi(argv[4]);
    data->isGroup             = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsMoveToColorCommandHandler(int argc, char ** argv)
{
    if (argc != 5)
    {
        return GroupsColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::MoveToColor::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->args[4]             = atoi(argv[4]);
    data->isGroup             = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsMoveColorCommandHandler(int argc, char ** argv)
{
    if (argc != 4)
    {
        return GroupsColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::MoveColor::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->isGroup             = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsStepColorCommandHandler(int argc, char ** argv)
{
    if (argc != 5)
    {
        return GroupsColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::MoveColor::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->args[4]             = atoi(argv[4]);
    data->isGroup             = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsMoveToColorTemperatureCommandHandler(int argc, char ** argv)
{
    if (argc != 4)
    {
        return GroupsColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::MoveColor::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->isGroup             = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsEnhancedMoveToHueCommandHandler(int argc, char ** argv)
{
    if (argc != 5)
    {
        return GroupsColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::EnhancedMoveToHue::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->args[4]             = atoi(argv[4]);
    data->isGroup             = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsEnhancedMoveHueCommandHandler(int argc, char ** argv)
{
    if (argc != 4)
    {
        return GroupsColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::EnhancedMoveHue::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->isGroup             = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsEnhancedStepHueCommandHandler(int argc, char ** argv)
{
    if (argc != 5)
    {
        return GroupsColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::EnhancedStepHue::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->args[4]             = atoi(argv[4]);
    data->isGroup             = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsEnhancedMoveToHueAndSaturationCommandHandler(int argc, char ** argv)
{
    if (argc != 5)
    {
        return GroupsColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::EnhancedMoveToHueAndSaturation::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->args[4]             = atoi(argv[4]);
    data->isGroup             = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsColorLoopSetCommandHandler(int argc, char ** argv)
{
    if (argc != 7)
    {
        return GroupsColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::ColorLoopSet::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->args[4]             = atoi(argv[4]);
    data->args[5]             = atoi(argv[5]);
    data->args[6]             = atoi(argv[6]);
    data->isGroup             = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsStopMoveStepCommandHandler(int argc, char ** argv)
{
    if (argc != 2)
    {
        return GroupsColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::StopMoveStep::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->isGroup             = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsMoveColorTemperatureCommandHandler(int argc, char ** argv)
{
    if (argc != 6)
    {
        return GroupsColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::MoveColorTemperature::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->args[4]             = atoi(argv[4]);
    data->args[5]             = atoi(argv[5]);
    data->isGroup             = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsStepColorTemperatureCommandHandler(int argc, char ** argv)
{
    if (argc != 7)
    {
        return GroupsColorControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::ColorControl::Commands::StepColorTemperature::Id;
    data->clusterId           = chip::app::Clusters::ColorControl::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->args[4]             = atoi(argv[4]);
    data->args[5]             = atoi(argv[5]);
    data->args[6]             = atoi(argv[6]);
    data->isGroup             = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}
#endif // CONFIG_ENABLE_CHIP_SHELL
