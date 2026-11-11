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

#include <posix/named_pipe/translators/ClosureControllerTranslator.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>

#include <optional>
#include <string>

namespace chip::app::NamedPipe {

namespace {

constexpr uint8_t kDefaultEffectVariant = 0;

CHIP_ERROR DispatchTriggerEffect(OOBAccessorRegistry & registry, EndpointId endpointId, uint8_t effectIdentifier,
                                 uint8_t effectVariant)
{
    uint8_t buffer[32];
    TLV::TLVWriter writer;
    writer.Init(buffer, sizeof(buffer));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(1), endpointId));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(2), effectIdentifier));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(3), effectVariant));
    ReturnErrorOnFailure(writer.EndContainer(outerType));
    ReturnErrorOnFailure(writer.Finalize());

    return registry.HandleAction("ClosureTriggerEffect"_span, ByteSpan(buffer, writer.GetLengthWritten()));
}

/// Encodes MoveTo, GroupedMoveTo, SetTarget and GroupedSetTarget. Absent fields are not written.
CHIP_ERROR DispatchPositionAction(OOBAccessorRegistry & registry, CharSpan actionName, EndpointId endpointId,
                                  std::optional<uint16_t> position, std::optional<bool> latch, std::optional<uint8_t> speed)
{
    uint8_t buffer[32];
    TLV::TLVWriter writer;
    writer.Init(buffer, sizeof(buffer));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(1), endpointId));
    if (position.has_value())
    {
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(2), *position));
    }
    if (speed.has_value())
    {
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(4), *speed));
    }
    if (latch.has_value())
    {
        ReturnErrorOnFailure(writer.PutBoolean(TLV::ContextTag(5), *latch));
    }
    ReturnErrorOnFailure(writer.EndContainer(outerType));
    ReturnErrorOnFailure(writer.Finalize());

    return registry.HandleAction(actionName, ByteSpan(buffer, writer.GetLengthWritten()));
}

/// Encodes Step and GroupedStep. Speed is not written when absent.
CHIP_ERROR DispatchStepAction(OOBAccessorRegistry & registry, CharSpan actionName, EndpointId endpointId, uint8_t direction,
                              uint16_t numberOfSteps, std::optional<uint8_t> speed)
{
    uint8_t buffer[32];
    TLV::TLVWriter writer;
    writer.Init(buffer, sizeof(buffer));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(1), endpointId));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(2), direction));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(3), numberOfSteps));
    if (speed.has_value())
    {
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(4), *speed));
    }
    ReturnErrorOnFailure(writer.EndContainer(outerType));
    ReturnErrorOnFailure(writer.Finalize());

    return registry.HandleAction(actionName, ByteSpan(buffer, writer.GetLengthWritten()));
}

} // namespace

CHIP_ERROR ClosureControllerTranslator::TranslateAndExecute(EndpointId endpointId, const Json::Value & json,
                                                            OOBAccessorRegistry & registry) const
{
    if (!json.isObject() || !json.isMember("Name") || !json["Name"].isString())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    std::string action = json["Name"].asString();
    if (action == "ClosureMoveTo" || action == "ClosureGroupedMoveTo" || action == "ClosureSetTarget" ||
        action == "ClosureGroupedSetTarget")
    {
        auto position = ExtractUInt<uint16_t>(json, "Position");
        VerifyOrReturnError(position.has_value() || !json.isMember("Position"), CHIP_ERROR_INVALID_ARGUMENT);
        auto latch = ExtractBool(json, "Latch");
        VerifyOrReturnError(latch.has_value() || !json.isMember("Latch"), CHIP_ERROR_INVALID_ARGUMENT);
        auto speed = ExtractUInt<uint8_t>(json, "Speed");
        VerifyOrReturnError(speed.has_value() || !json.isMember("Speed"), CHIP_ERROR_INVALID_ARGUMENT);
        return DispatchPositionAction(registry, CharSpan(action.data(), action.size()), endpointId, position, latch, speed);
    }
    if (action == "ClosureStep" || action == "ClosureGroupedStep")
    {
        auto direction     = ExtractUInt<uint8_t>(json, "Direction");
        auto numberOfSteps = ExtractUInt<uint16_t>(json, "NumberOfSteps");
        VerifyOrReturnError(direction.has_value() && numberOfSteps.has_value(), CHIP_ERROR_INVALID_ARGUMENT);
        auto speed = ExtractUInt<uint8_t>(json, "Speed");
        VerifyOrReturnError(speed.has_value() || !json.isMember("Speed"), CHIP_ERROR_INVALID_ARGUMENT);
        return DispatchStepAction(registry, CharSpan(action.data(), action.size()), endpointId, *direction, *numberOfSteps, speed);
    }
    if (action == "ClosureStop")
    {
        return DispatchAction(registry, "ClosureStop"_span, endpointId);
    }
    if (action == "ClosureCalibrate")
    {
        return DispatchAction(registry, "ClosureCalibrate"_span, endpointId);
    }
    if (action == "ClosureIdentify")
    {
        auto identifyTime = ExtractUInt<uint16_t>(json, "IdentifyTime");
        VerifyOrReturnError(identifyTime.has_value(), CHIP_ERROR_INVALID_ARGUMENT);
        return DispatchAction(registry, "ClosureIdentify"_span, endpointId, *identifyTime);
    }
    if (action == "ClosureTriggerEffect")
    {
        auto effectIdentifier = ExtractUInt<uint8_t>(json, "EffectIdentifier");
        VerifyOrReturnError(effectIdentifier.has_value(), CHIP_ERROR_INVALID_ARGUMENT);
        auto effectVariant = ExtractUInt<uint8_t>(json, "EffectVariant");
        VerifyOrReturnError(effectVariant.has_value() || !json.isMember("EffectVariant"), CHIP_ERROR_INVALID_ARGUMENT);
        return DispatchTriggerEffect(registry, endpointId, *effectIdentifier, effectVariant.value_or(kDefaultEffectVariant));
    }
    return CHIP_ERROR_NOT_FOUND;
}

} // namespace chip::app::NamedPipe
