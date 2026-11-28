/*
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
#include <clusters/WindowCovering/Commands.h>
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

enum class WindowCoveringAction : uint8_t
{
    kUpOrOpen,
    kDownOrClose,
    kStopMotion,
    kGoToLiftPercentage,
    kGoToTiltPercentage,
};

std::optional<WindowCoveringAction> WindowCoveringActionFromName(CharSpan name)
{
    if (name.data_equal("WindowCoveringUpOrOpen"_span))
    {
        return WindowCoveringAction::kUpOrOpen;
    }
    if (name.data_equal("WindowCoveringDownOrClose"_span))
    {
        return WindowCoveringAction::kDownOrClose;
    }
    if (name.data_equal("WindowCoveringStopMotion"_span))
    {
        return WindowCoveringAction::kStopMotion;
    }
    if (name.data_equal("WindowCoveringGoToLiftPercentage"_span))
    {
        return WindowCoveringAction::kGoToLiftPercentage;
    }
    if (name.data_equal("WindowCoveringGoToTiltPercentage"_span))
    {
        return WindowCoveringAction::kGoToTiltPercentage;
    }
    return std::nullopt;
}

/// Decoded TLV payload. Tag 2 is only present for the GoTo actions (see OOBAccessors.h).
struct WindowCoveringActionArguments
{
    EndpointId endpointId = kInvalidEndpointId;
    std::optional<Percent100ths> percent100ths;
};

CHIP_ERROR ParseArguments(ByteSpan tlvData, WindowCoveringActionArguments & out)
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
            Percent100ths value;
            ReturnErrorOnFailure(reader.Get(value));
            out.percent100ths = value;
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

class WindowCoveringControllerOOBAccessor : public OOBAccessor
{
public:
    explicit WindowCoveringControllerOOBAccessor(WindowCoveringController & device) : mDevice(device) {}

    std::optional<CHIP_ERROR> HandleAction(CharSpan action, ByteSpan tlvData) override
    {
        std::optional<WindowCoveringAction> windowCoveringAction = WindowCoveringActionFromName(action);
        if (!windowCoveringAction.has_value())
        {
            return std::nullopt;
        }

        WindowCoveringActionArguments arguments;
        ReturnErrorOnFailure(ParseArguments(tlvData, arguments));
        if (arguments.endpointId != mDevice.GetEndpointId())
        {
            return std::nullopt;
        }
        return Execute(*windowCoveringAction, arguments);
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

    CHIP_ERROR Execute(WindowCoveringAction action, const WindowCoveringActionArguments & arguments)
    {
        switch (action)
        {
        case WindowCoveringAction::kUpOrOpen:
            return DispatchCommand(WindowCovering::Commands::UpOrOpen::Type{});
        case WindowCoveringAction::kDownOrClose:
            return DispatchCommand(WindowCovering::Commands::DownOrClose::Type{});
        case WindowCoveringAction::kStopMotion:
            return DispatchCommand(WindowCovering::Commands::StopMotion::Type{});
        case WindowCoveringAction::kGoToLiftPercentage: {
            VerifyOrReturnError(arguments.percent100ths.has_value() && *arguments.percent100ths <= kMaxPercent100ths,
                                CHIP_ERROR_INVALID_ARGUMENT);
            WindowCovering::Commands::GoToLiftPercentage::Type cmd;
            cmd.liftPercent100thsValue = *arguments.percent100ths;
            return DispatchCommand(std::move(cmd));
        }
        case WindowCoveringAction::kGoToTiltPercentage: {
            VerifyOrReturnError(arguments.percent100ths.has_value() && *arguments.percent100ths <= kMaxPercent100ths,
                                CHIP_ERROR_INVALID_ARGUMENT);
            WindowCovering::Commands::GoToTiltPercentage::Type cmd;
            cmd.tiltPercent100thsValue = *arguments.percent100ths;
            return DispatchCommand(std::move(cmd));
        }
        }
        return CHIP_ERROR_INTERNAL;
    }

    WindowCoveringController & mDevice;
};

} // namespace

void RegisterOOBAccessors(WindowCoveringController & device, OOBAccessorRegistry & registry)
{
    LogErrorOnFailure(registry.Register(std::make_unique<WindowCoveringControllerOOBAccessor>(device)));
}

} // namespace chip::app
