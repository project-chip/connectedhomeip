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

#include <posix/named_pipe/translators/ProximityRangingTranslator.h>

#include <app/data-model/Encode.h>
#include <clusters/ProximityRanging/Enums.h>
#include <clusters/ProximityRanging/Structs.h>

#include <vector>

namespace chip::app::NamedPipe {

namespace {

using namespace chip::app::Clusters::ProximityRanging;

constexpr uint8_t kTagEndpointId  = 1;
constexpr uint8_t kTagConstraints = 2;

/// Save room for Up to 6 fields of at most 6 bytes each and the container.
constexpr size_t kBytesPerConstraint = 48;

/// Parses JSON constraint objects
std::optional<Structs::RangingConstraintStruct::Type> ParseConstraint(const Json::Value & json)
{
    VerifyOrReturnValue(json.isObject(), std::nullopt);

    Structs::RangingConstraintStruct::Type entry;

    auto technology = CommandTranslator::ExtractUInt<uint8_t>(json, "Technology");
    auto role       = CommandTranslator::ExtractUInt<uint8_t>(json, "Role");
    VerifyOrReturnValue(technology.has_value() && role.has_value(), std::nullopt);
    VerifyOrReturnValue(*technology < static_cast<uint8_t>(RangingTechEnum::kUnknownEnumValue), std::nullopt);
    VerifyOrReturnValue(*role < static_cast<uint8_t>(RangingRoleEnum::kUnknownEnumValue), std::nullopt);
    entry.technology = static_cast<RangingTechEnum>(*technology);
    entry.role       = static_cast<RangingRoleEnum>(*role);

    if (json.isMember("Enabled"))
    {
        auto enabled = CommandTranslator::ExtractBool(json, "Enabled");
        VerifyOrReturnValue(enabled.has_value(), std::nullopt);
        entry.enabled.SetValue(*enabled);
    }
    if (json.isMember("MinRangingInterval"))
    {
        auto value = CommandTranslator::ExtractUInt<uint32_t>(json, "MinRangingInterval");
        VerifyOrReturnValue(value.has_value(), std::nullopt);
        entry.minRangingInterval.SetValue(*value);
    }
    if (json.isMember("MaxSessionDuration"))
    {
        auto value = CommandTranslator::ExtractUInt<uint32_t>(json, "MaxSessionDuration");
        VerifyOrReturnValue(value.has_value(), std::nullopt);
        entry.maxSessionDuration.SetValue(*value);
    }
    if (json.isMember("MaxRangingInstances"))
    {
        auto value = CommandTranslator::ExtractUInt<uint16_t>(json, "MaxRangingInstances");
        VerifyOrReturnValue(value.has_value(), std::nullopt);
        entry.maxRangingInstances.SetValue(*value);
    }
    return entry;
}

} // namespace

CHIP_ERROR ProximityRangingTranslator::TranslateAndExecute(EndpointId endpointId, const Json::Value & json,
                                                           OOBAccessorRegistry & registry) const
{
    std::string actionName = json["Name"].asString();
    if (actionName == "SetRangingConstraints")
        return TranslateSetRangingConstraints(endpointId, json, registry);

    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR ProximityRangingTranslator::TranslateSetRangingConstraints(EndpointId endpointId, const Json::Value & json,
                                                                      OOBAccessorRegistry & registry) const
{
    const Json::Value & constraintsJson = json["Constraints"];
    VerifyOrReturnError(constraintsJson.isArray(), CHIP_ERROR_INVALID_ARGUMENT);

    std::vector<Structs::RangingConstraintStruct::Type> constraints;
    for (const auto & item : constraintsJson)
    {
        auto entry = ParseConstraint(item);
        VerifyOrReturnError(entry.has_value(), CHIP_ERROR_INVALID_ARGUMENT);
        constraints.push_back(*entry);
    }

    TlvMessageBuffer message(constraints.size() * kBytesPerConstraint);
    TLV::TLVWriter & writer = message.Writer();

    TLV::TLVType outerType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kTagEndpointId), endpointId));

    TLV::TLVType arrayType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(kTagConstraints), TLV::kTLVType_Array, arrayType));
    for (const auto & entry : constraints)
        ReturnErrorOnFailure(DataModel::Encode(writer, TLV::AnonymousTag(), entry));
    ReturnErrorOnFailure(writer.EndContainer(arrayType));
    ReturnErrorOnFailure(writer.EndContainer(outerType));

    ByteSpan payload;
    ReturnErrorOnFailure(message.Finalize(payload));
    return registry.HandleAction("SetRangingConstraints"_span, payload);
}

} // namespace chip::app::NamedPipe
