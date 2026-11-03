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

#include <clusters/Identify/Enums.h>
#include <device/types/on-off-light-switch/OnOffLightSwitch.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/Span.h>
#include <oob-accessors/OOBAccessor.h>

#include <memory>
#include <optional>

namespace chip::app {

namespace {

using Clusters::Identify::EffectIdentifierEnum;
using Clusters::Identify::EffectVariantEnum;

enum class SwitchAction : uint8_t
{
    kOn,
    kOff,
    kToggle,
    kIdentify,
    kTriggerEffect,
};

std::optional<SwitchAction> SwitchActionFromName(CharSpan name)
{
    if (name.data_equal("SwitchOn"_span))
    {
        return SwitchAction::kOn;
    }
    if (name.data_equal("SwitchOff"_span))
    {
        return SwitchAction::kOff;
    }
    if (name.data_equal("SwitchToggle"_span))
    {
        return SwitchAction::kToggle;
    }
    if (name.data_equal("SwitchIdentify"_span))
    {
        return SwitchAction::kIdentify;
    }
    if (name.data_equal("SwitchTriggerEffect"_span))
    {
        return SwitchAction::kTriggerEffect;
    }
    return std::nullopt;
}

/// Decoded TLV payload. The meaning of tags 2 and 3 depends on the action (see OOBAccessors.h).
struct SwitchActionArguments
{
    EndpointId endpointId = kInvalidEndpointId;
    std::optional<uint16_t> tag2;
    std::optional<uint8_t> tag3;
};

CHIP_ERROR ParseArguments(ByteSpan tlvData, SwitchActionArguments & out)
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
            uint16_t value;
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
        default:
            break;
        }
    }
    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
    ReturnErrorOnFailure(reader.ExitContainer(outerType));
    VerifyOrReturnError(hasEndpointId, CHIP_ERROR_INVALID_ARGUMENT);
    return CHIP_NO_ERROR;
}

class OnOffLightSwitchOOBAccessor : public OOBAccessor
{
public:
    explicit OnOffLightSwitchOOBAccessor(OnOffLightSwitch & device) : mDevice(device) {}

    std::optional<CHIP_ERROR> HandleAction(CharSpan action, ByteSpan tlvData) override
    {
        std::optional<SwitchAction> switchAction = SwitchActionFromName(action);
        if (!switchAction.has_value())
        {
            return std::nullopt;
        }

        SwitchActionArguments arguments;
        ReturnErrorOnFailure(ParseArguments(tlvData, arguments));
        if (arguments.endpointId != mDevice.GetEndpointId())
        {
            return std::nullopt;
        }
        return Execute(*switchAction, arguments);
    }

private:
    CHIP_ERROR Execute(SwitchAction action, const SwitchActionArguments & arguments)
    {
        switch (action)
        {
        case SwitchAction::kOn:
            return mDevice.SendOn();
        case SwitchAction::kOff:
            return mDevice.SendOff();
        case SwitchAction::kToggle:
            return mDevice.SendToggle();
        case SwitchAction::kIdentify:
            VerifyOrReturnError(arguments.tag2.has_value(), CHIP_ERROR_INVALID_ARGUMENT);
            return mDevice.SendIdentify(*arguments.tag2);
        case SwitchAction::kTriggerEffect:
            VerifyOrReturnError(arguments.tag2.has_value() && arguments.tag3.has_value(), CHIP_ERROR_INVALID_ARGUMENT);
            VerifyOrReturnError(CanCastTo<uint8_t>(*arguments.tag2), CHIP_ERROR_INVALID_ARGUMENT);
            return mDevice.SendTriggerEffect(static_cast<EffectIdentifierEnum>(*arguments.tag2),
                                             static_cast<EffectVariantEnum>(*arguments.tag3));
        }
        return CHIP_ERROR_INTERNAL;
    }

    OnOffLightSwitch & mDevice;
};

} // namespace

void RegisterOOBAccessors(OnOffLightSwitch & device, OOBAccessorRegistry & registry)
{
    LogErrorOnFailure(registry.Register(std::make_unique<OnOffLightSwitchOOBAccessor>(device)));
}

} // namespace chip::app
