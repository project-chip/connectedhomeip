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
#include <clusters/Thermostat/Commands.h>
#include <clusters/Thermostat/Enums.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <oob-accessors/OOBAccessor.h>

#include <memory>
#include <optional>

namespace chip::app {

namespace {

using SetpointRaiseLower = Clusters::Thermostat::Commands::SetpointRaiseLower::Type;

constexpr CharSpan kSetpointRaiseLowerAction = "HeatPumpSetpointRaiseLower"_span;

struct SetpointRaiseLowerArguments
{
    EndpointId endpointId = kInvalidEndpointId;
    std::optional<uint8_t> mode;
    std::optional<int8_t> amount;
};

CHIP_ERROR ParseArguments(ByteSpan tlvData, SetpointRaiseLowerArguments & out)
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
            out.mode = value;
            break;
        }
        case 3: {
            int8_t value;
            ReturnErrorOnFailure(reader.Get(value));
            out.amount = value;
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

class HeatPumpOOBAccessor : public OOBAccessor
{
public:
    explicit HeatPumpOOBAccessor(SimulatedHeatPump & device) : mDevice(device) {}

    std::optional<CHIP_ERROR> HandleAction(CharSpan action, ByteSpan tlvData) override
    {
        if (!action.data_equal(kSetpointRaiseLowerAction))
        {
            return std::nullopt;
        }

        SetpointRaiseLowerArguments arguments;
        ReturnErrorOnFailure(ParseArguments(tlvData, arguments));
        if (arguments.endpointId != mDevice.GetEndpointId())
        {
            return std::nullopt;
        }
        return SendSetpointRaiseLower(arguments);
    }

private:
    CHIP_ERROR SendSetpointRaiseLower(const SetpointRaiseLowerArguments & arguments)
    {
        VerifyOrReturnError(arguments.mode.has_value() && arguments.amount.has_value(), CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(*arguments.mode <= to_underlying(Clusters::Thermostat::SetpointRaiseLowerModeEnum::kBoth),
                            CHIP_IM_GLOBAL_STATUS(ConstraintError));

        SetpointRaiseLower command;
        command.mode   = static_cast<Clusters::Thermostat::SetpointRaiseLowerModeEnum>(*arguments.mode);
        command.amount = *arguments.amount;

        Platform::UniquePtr<BindingCommand> bindingCommand(Platform::New<SimpleBindingCommand<SetpointRaiseLower>>(command));
        VerifyOrReturnError(bindingCommand != nullptr, CHIP_ERROR_NO_MEMORY);
        return mDevice.Dispatch(std::move(bindingCommand));
    }

    SimulatedHeatPump & mDevice;
};

} // namespace

void RegisterOOBAccessors(SimulatedHeatPump & device, OOBAccessorRegistry & registry)
{
    LogErrorOnFailure(registry.Register(std::make_unique<HeatPumpOOBAccessor>(device)));
}

} // namespace chip::app
