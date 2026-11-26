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

#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <oob-accessors/OOBAccessor.h>

#include <memory>
#include <optional>

namespace chip::app {

namespace {

enum class SensorAction : uint8_t
{
    kOn,
    kOff,
    kToggle,
    kIdentify,
};

std::optional<SensorAction> SensorActionFromName(CharSpan name)
{
    if (name.data_equal("SensorOn"_span))
    {
        return SensorAction::kOn;
    }
    if (name.data_equal("SensorOff"_span))
    {
        return SensorAction::kOff;
    }
    if (name.data_equal("SensorToggle"_span))
    {
        return SensorAction::kToggle;
    }
    if (name.data_equal("SensorIdentify"_span))
    {
        return SensorAction::kIdentify;
    }
    return std::nullopt;
}

struct SensorActionArguments
{
    EndpointId endpointId = kInvalidEndpointId;
    std::optional<uint16_t> identifyTime;
};

CHIP_ERROR ParseArguments(ByteSpan tlvData, SensorActionArguments & out)
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
            out.identifyTime = value;
            break;
        }
        default:
            break;
        }
    }
    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
    ReturnErrorOnFailure(reader.ExitContainer(outerType));
    VerifyOrReturnError(hasEndpointId && out.endpointId != kInvalidEndpointId, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(reader.Next() == CHIP_END_OF_TLV, CHIP_ERROR_INVALID_TLV_ELEMENT);
    return CHIP_NO_ERROR;
}

class OnOffSensorOOBAccessor : public OOBAccessor
{
public:
    explicit OnOffSensorOOBAccessor(OnOffSensor & device) : mDevice(device) {}

    std::optional<CHIP_ERROR> HandleAction(CharSpan action, ByteSpan tlvData) override
    {
        std::optional<SensorAction> sensorAction = SensorActionFromName(action);
        if (!sensorAction.has_value())
        {
            return std::nullopt;
        }

        SensorActionArguments arguments;
        ReturnErrorOnFailure(ParseArguments(tlvData, arguments));
        if (arguments.endpointId != mDevice.GetEndpointId())
        {
            return std::nullopt;
        }
        return Execute(*sensorAction, arguments);
    }

private:
    CHIP_ERROR Execute(SensorAction action, const SensorActionArguments & arguments)
    {
        switch (action)
        {
        case SensorAction::kOn:
            return mDevice.SendOn();
        case SensorAction::kOff:
            return mDevice.SendOff();
        case SensorAction::kToggle:
            return mDevice.SendToggle();
        case SensorAction::kIdentify:
            VerifyOrReturnError(arguments.identifyTime.has_value(), CHIP_ERROR_INVALID_ARGUMENT);
            return mDevice.SendIdentify(*arguments.identifyTime);
        }
        return CHIP_ERROR_INTERNAL;
    }

    OnOffSensor & mDevice;
};

} // namespace

void RegisterOOBAccessors(OnOffSensor & device, OOBAccessorRegistry & registry)
{
    LogErrorOnFailure(registry.Register(std::make_unique<OnOffSensorOOBAccessor>(device)));
}

} // namespace chip::app
