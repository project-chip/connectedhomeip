/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "OOBAccessors.h"

#include <binding-handler/SimpleBindingCommand.h>
#include <clusters/ClosureControl/Commands.h>
#include <clusters/ClosureControl/EnumsCheck.h>
#include <clusters/ClosureDimension/Commands.h>
#include <clusters/ClosureDimension/EnumsCheck.h>
#include <clusters/Identify/Commands.h>
#include <clusters/Identify/EnumsCheck.h>
#include <clusters/shared/EnumsCheck.h>
#include <device/types/closure-controller/ClosureController.h>
#include <lib/core/TLV.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <oob-accessors/OOBAccessor.h>

#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

namespace chip::app {

namespace {

using namespace Clusters;

constexpr Percent100ths kMaxPercent100ths = 10000;

enum class ClosureAction : uint8_t
{
    kMoveTo,
    kGroupedMoveTo,
    kSetTarget,
    kGroupedSetTarget,
    kStep,
    kGroupedStep,
    kStop,
    kCalibrate,
    kIdentify,
    kTriggerEffect,
};

std::optional<ClosureAction> ClosureActionFromName(CharSpan name)
{
    if (name.data_equal("ClosureMoveTo"_span))
    {
        return ClosureAction::kMoveTo;
    }
    if (name.data_equal("ClosureGroupedMoveTo"_span))
    {
        return ClosureAction::kGroupedMoveTo;
    }
    if (name.data_equal("ClosureSetTarget"_span))
    {
        return ClosureAction::kSetTarget;
    }
    if (name.data_equal("ClosureGroupedSetTarget"_span))
    {
        return ClosureAction::kGroupedSetTarget;
    }
    if (name.data_equal("ClosureStep"_span))
    {
        return ClosureAction::kStep;
    }
    if (name.data_equal("ClosureGroupedStep"_span))
    {
        return ClosureAction::kGroupedStep;
    }
    if (name.data_equal("ClosureStop"_span))
    {
        return ClosureAction::kStop;
    }
    if (name.data_equal("ClosureCalibrate"_span))
    {
        return ClosureAction::kCalibrate;
    }
    if (name.data_equal("ClosureIdentify"_span))
    {
        return ClosureAction::kIdentify;
    }
    if (name.data_equal("ClosureTriggerEffect"_span))
    {
        return ClosureAction::kTriggerEffect;
    }
    return std::nullopt;
}

/// Decoded TLV payload. The meaning of tags 2 to 5 depends on the action (see OOBAccessors.h).
struct ClosureActionArguments
{
    EndpointId endpointId = kInvalidEndpointId;
    std::optional<uint8_t> tag2;
    std::optional<uint8_t> tag3;
    std::optional<uint16_t> tag4;
    std::optional<bool> tag5;
};

CHIP_ERROR ParseArguments(ByteSpan tlvData, ClosureActionArguments & out)
{
    TLV::TLVReader reader;
    reader.Init(tlvData);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(reader.EnterContainer(outerType));

    bool hasEndpointId = false;
    CHIP_ERROR err     = CHIP_NO_ERROR;
    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        TLV::Tag tag = reader.GetTag();
        if (!TLV::IsContextTag(tag))
        {
            continue;
        }
        switch (TLV::TagNumFromTag(tag))
        {
        case 1:
            ReturnErrorOnFailure(reader.Get(out.endpointId));
            hasEndpointId = true;
            break;
        case 2: {
            uint8_t value;
            ReturnErrorOnFailure(reader.Get(value));
            out.tag2 = value;
            break;
        }
        case 3: {
            uint8_t value;
            ReturnErrorOnFailure(reader.Get(value));
            out.tag3 = value;
            break;
        }
        case 4: {
            uint16_t value;
            ReturnErrorOnFailure(reader.Get(value));
            out.tag4 = value;
            break;
        }
        case 5: {
            bool value;
            ReturnErrorOnFailure(reader.Get(value));
            out.tag5 = value;
            break;
        }
        default:
            break;
        }
    }
    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
    ReturnErrorOnFailure(reader.ExitContainer(outerType));
    VerifyOrReturnError(hasEndpointId, CHIP_ERROR_INVALID_ARGUMENT);
    return CHIP_NO_ERROR;
}

CHIP_ERROR SetSpeed(std::optional<uint8_t> tag, Optional<Globals::ThreeLevelAutoEnum> & out)
{
    if (tag.has_value())
    {
        auto speed = static_cast<Globals::ThreeLevelAutoEnum>(*tag);
        VerifyOrReturnError(EnsureKnownEnumValue(speed) != Globals::ThreeLevelAutoEnum::kUnknownEnumValue,
                            CHIP_ERROR_INVALID_ARGUMENT);
        out.SetValue(speed);
    }
    return CHIP_NO_ERROR;
}

/// Fills ClosureControl MoveTo or GroupedMoveTo.
template <typename CommandType>
CHIP_ERROR BuildMoveTo(const ClosureActionArguments & arguments, CommandType & cmd)
{
    if (arguments.tag2.has_value())
    {
        auto position = static_cast<ClosureControl::TargetPositionEnum>(*arguments.tag2);
        VerifyOrReturnError(EnsureKnownEnumValue(position) != ClosureControl::TargetPositionEnum::kUnknownEnumValue,
                            CHIP_ERROR_INVALID_ARGUMENT);
        cmd.position.SetValue(position);
    }
    ReturnErrorOnFailure(SetSpeed(arguments.tag3, cmd.speed));
    if (arguments.tag5.has_value())
    {
        cmd.latch.SetValue(*arguments.tag5);
    }
    return CHIP_NO_ERROR;
}

/// Fills ClosureDimension SetTarget or GroupedSetTarget.
template <typename CommandType>
CHIP_ERROR BuildSetTarget(const ClosureActionArguments & arguments, CommandType & cmd)
{
    if (arguments.tag4.has_value())
    {
        VerifyOrReturnError(*arguments.tag4 <= kMaxPercent100ths, CHIP_ERROR_INVALID_ARGUMENT);
        cmd.position.SetValue(*arguments.tag4);
    }
    ReturnErrorOnFailure(SetSpeed(arguments.tag3, cmd.speed));
    if (arguments.tag5.has_value())
    {
        cmd.latch.SetValue(*arguments.tag5);
    }
    return CHIP_NO_ERROR;
}

/// Fills ClosureDimension Step or GroupedStep.
template <typename CommandType>
CHIP_ERROR BuildStep(const ClosureActionArguments & arguments, CommandType & cmd)
{
    VerifyOrReturnError(arguments.tag2.has_value() && arguments.tag4.has_value(), CHIP_ERROR_INVALID_ARGUMENT);
    auto direction = static_cast<ClosureDimension::StepDirectionEnum>(*arguments.tag2);
    VerifyOrReturnError(EnsureKnownEnumValue(direction) != ClosureDimension::StepDirectionEnum::kUnknownEnumValue,
                        CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(*arguments.tag4 >= 1, CHIP_ERROR_INVALID_ARGUMENT);
    cmd.direction     = direction;
    cmd.numberOfSteps = *arguments.tag4;
    return SetSpeed(arguments.tag3, cmd.speed);
}

class ClosureControllerOOBAccessor : public OOBAccessor
{
public:
    explicit ClosureControllerOOBAccessor(ClosureController & device) : mDevice(device) {}

    std::optional<CHIP_ERROR> HandleAction(CharSpan action, ByteSpan tlvData) override
    {
        std::optional<ClosureAction> closureAction = ClosureActionFromName(action);
        if (!closureAction.has_value())
        {
            return std::nullopt;
        }

        ClosureActionArguments arguments;
        ReturnErrorOnFailure(ParseArguments(tlvData, arguments));
        if (arguments.endpointId != mDevice.GetEndpointId())
        {
            return std::nullopt;
        }
        return Execute(*closureAction, arguments);
    }

private:
    template <typename CommandType>
    CHIP_ERROR DispatchCommand(CommandType && cmd)
    {
        Platform::UniquePtr<BindingCommand> command(
            Platform::New<SimpleBindingCommand<std::decay_t<CommandType>>>(std::forward<CommandType>(cmd)));
        VerifyOrReturnError(command != nullptr, CHIP_ERROR_NO_MEMORY);
        return mDevice.Dispatch(std::move(command));
    }

    CHIP_ERROR Execute(ClosureAction action, const ClosureActionArguments & arguments)
    {
        switch (action)
        {
        case ClosureAction::kMoveTo: {
            ClosureControl::Commands::MoveTo::Type cmd;
            ReturnErrorOnFailure(BuildMoveTo(arguments, cmd));
            return DispatchCommand(std::move(cmd));
        }
        case ClosureAction::kGroupedMoveTo: {
            ClosureControl::Commands::GroupedMoveTo::Type cmd;
            ReturnErrorOnFailure(BuildMoveTo(arguments, cmd));
            return DispatchCommand(std::move(cmd));
        }
        case ClosureAction::kSetTarget: {
            ClosureDimension::Commands::SetTarget::Type cmd;
            ReturnErrorOnFailure(BuildSetTarget(arguments, cmd));
            return DispatchCommand(std::move(cmd));
        }
        case ClosureAction::kGroupedSetTarget: {
            ClosureDimension::Commands::GroupedSetTarget::Type cmd;
            ReturnErrorOnFailure(BuildSetTarget(arguments, cmd));
            return DispatchCommand(std::move(cmd));
        }
        case ClosureAction::kStep: {
            ClosureDimension::Commands::Step::Type cmd;
            ReturnErrorOnFailure(BuildStep(arguments, cmd));
            return DispatchCommand(std::move(cmd));
        }
        case ClosureAction::kGroupedStep: {
            ClosureDimension::Commands::GroupedStep::Type cmd;
            ReturnErrorOnFailure(BuildStep(arguments, cmd));
            return DispatchCommand(std::move(cmd));
        }
        case ClosureAction::kStop:
            return DispatchCommand(ClosureControl::Commands::Stop::Type{});
        case ClosureAction::kCalibrate:
            return DispatchCommand(ClosureControl::Commands::Calibrate::Type{});
        case ClosureAction::kIdentify: {
            VerifyOrReturnError(arguments.tag4.has_value(), CHIP_ERROR_INVALID_ARGUMENT);
            Identify::Commands::Identify::Type cmd;
            cmd.identifyTime = *arguments.tag4;
            return DispatchCommand(std::move(cmd));
        }
        case ClosureAction::kTriggerEffect: {
            VerifyOrReturnError(arguments.tag2.has_value() && arguments.tag3.has_value(), CHIP_ERROR_INVALID_ARGUMENT);
            Identify::Commands::TriggerEffect::Type cmd;
            cmd.effectIdentifier = static_cast<Identify::EffectIdentifierEnum>(*arguments.tag2);
            cmd.effectVariant    = static_cast<Identify::EffectVariantEnum>(*arguments.tag3);
            VerifyOrReturnError(EnsureKnownEnumValue(cmd.effectIdentifier) != Identify::EffectIdentifierEnum::kUnknownEnumValue,
                                CHIP_ERROR_INVALID_ARGUMENT);
            VerifyOrReturnError(EnsureKnownEnumValue(cmd.effectVariant) != Identify::EffectVariantEnum::kUnknownEnumValue,
                                CHIP_ERROR_INVALID_ARGUMENT);
            return DispatchCommand(std::move(cmd));
        }
        }
        return CHIP_ERROR_INTERNAL;
    }

    ClosureController & mDevice;
};

} // namespace

void RegisterOOBAccessors(ClosureController & device, OOBAccessorRegistry & registry)
{
    LogErrorOnFailure(registry.Register(std::make_unique<ClosureControllerOOBAccessor>(device)));
}

} // namespace chip::app
