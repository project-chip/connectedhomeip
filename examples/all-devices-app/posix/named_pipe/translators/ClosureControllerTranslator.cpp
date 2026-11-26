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

/// Writes the closure controller action TLV and dispatches it. Absent fields are not written.
/// The meaning of each tag per action is documented in closure-controller/OOBAccessors.h.
CHIP_ERROR DispatchClosureAction(OOBAccessorRegistry & registry, CharSpan actionName, EndpointId endpointId,
                                 std::optional<uint8_t> tag2, std::optional<uint8_t> tag3, std::optional<uint16_t> tag4,
                                 std::optional<bool> tag5)
{
    uint8_t buffer[32];
    TLV::TLVWriter writer;
    writer.Init(buffer, sizeof(buffer));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(1), endpointId));
    if (tag2.has_value())
    {
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(2), *tag2));
    }
    if (tag3.has_value())
    {
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(3), *tag3));
    }
    if (tag4.has_value())
    {
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(4), *tag4));
    }
    if (tag5.has_value())
    {
        ReturnErrorOnFailure(writer.PutBoolean(TLV::ContextTag(5), *tag5));
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
    CharSpan actionName(action.data(), action.size());

    if (action == "ClosureMoveTo" || action == "ClosureGroupedMoveTo")
    {
        auto position = ExtractUInt<uint8_t>(json, "Position");
        VerifyOrReturnError(position.has_value() || !json.isMember("Position"), CHIP_ERROR_INVALID_ARGUMENT);
        auto speed = ExtractUInt<uint8_t>(json, "Speed");
        VerifyOrReturnError(speed.has_value() || !json.isMember("Speed"), CHIP_ERROR_INVALID_ARGUMENT);
        auto latch = ExtractBool(json, "Latch");
        VerifyOrReturnError(latch.has_value() || !json.isMember("Latch"), CHIP_ERROR_INVALID_ARGUMENT);
        return DispatchClosureAction(registry, actionName, endpointId, position, speed, std::nullopt, latch);
    }
    if (action == "ClosureSetTarget" || action == "ClosureGroupedSetTarget")
    {
        auto position = ExtractUInt<uint16_t>(json, "Position");
        VerifyOrReturnError(position.has_value() || !json.isMember("Position"), CHIP_ERROR_INVALID_ARGUMENT);
        auto speed = ExtractUInt<uint8_t>(json, "Speed");
        VerifyOrReturnError(speed.has_value() || !json.isMember("Speed"), CHIP_ERROR_INVALID_ARGUMENT);
        auto latch = ExtractBool(json, "Latch");
        VerifyOrReturnError(latch.has_value() || !json.isMember("Latch"), CHIP_ERROR_INVALID_ARGUMENT);
        return DispatchClosureAction(registry, actionName, endpointId, std::nullopt, speed, position, latch);
    }
    if (action == "ClosureStep" || action == "ClosureGroupedStep")
    {
        auto direction     = ExtractUInt<uint8_t>(json, "Direction");
        auto numberOfSteps = ExtractUInt<uint16_t>(json, "NumberOfSteps");
        VerifyOrReturnError(direction.has_value() && numberOfSteps.has_value(), CHIP_ERROR_INVALID_ARGUMENT);
        auto speed = ExtractUInt<uint8_t>(json, "Speed");
        VerifyOrReturnError(speed.has_value() || !json.isMember("Speed"), CHIP_ERROR_INVALID_ARGUMENT);
        return DispatchClosureAction(registry, actionName, endpointId, direction, speed, numberOfSteps, std::nullopt);
    }
    if (action == "ClosureStop" || action == "ClosureCalibrate")
    {
        return DispatchAction(registry, actionName, endpointId);
    }
    if (action == "ClosureIdentify")
    {
        auto identifyTime = ExtractUInt<uint16_t>(json, "IdentifyTime");
        VerifyOrReturnError(identifyTime.has_value(), CHIP_ERROR_INVALID_ARGUMENT);
        return DispatchClosureAction(registry, actionName, endpointId, std::nullopt, std::nullopt, identifyTime, std::nullopt);
    }
    if (action == "ClosureTriggerEffect")
    {
        auto effectIdentifier = ExtractUInt<uint8_t>(json, "EffectIdentifier");
        VerifyOrReturnError(effectIdentifier.has_value(), CHIP_ERROR_INVALID_ARGUMENT);
        auto effectVariant = ExtractUInt<uint8_t>(json, "EffectVariant");
        VerifyOrReturnError(effectVariant.has_value() || !json.isMember("EffectVariant"), CHIP_ERROR_INVALID_ARGUMENT);
        return DispatchClosureAction(registry, actionName, endpointId, effectIdentifier,
                                     effectVariant.value_or(kDefaultEffectVariant), std::nullopt, std::nullopt);
    }
    return CHIP_ERROR_NOT_FOUND;
}

} // namespace chip::app::NamedPipe
