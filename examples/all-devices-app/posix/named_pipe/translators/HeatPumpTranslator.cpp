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

#include <posix/named_pipe/translators/HeatPumpTranslator.h>

#include <cstdint>
#include <limits>
#include <optional>

namespace chip::app::NamedPipe {

namespace {

std::optional<int8_t> ExtractInt8(const Json::Value & json, const char * key)
{
    if (!json.isMember(key) || !json[key].isIntegral() || !json[key].isInt64())
    {
        return std::nullopt;
    }
    const int64_t value = json[key].asInt64();
    if (value < std::numeric_limits<int8_t>::min() || value > std::numeric_limits<int8_t>::max())
    {
        return std::nullopt;
    }
    return static_cast<int8_t>(value);
}

} // namespace

CHIP_ERROR HeatPumpTranslator::TranslateAndExecute(EndpointId endpointId, const Json::Value & json,
                                                   OOBAccessorRegistry & registry) const
{
    VerifyOrReturnError(json.isObject() && json.isMember("Name") && json["Name"].isString(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(json["Name"].asString() == "HeatPumpSetpointRaiseLower", CHIP_ERROR_NOT_FOUND);

    std::optional<uint8_t> mode  = ExtractUInt<uint8_t>(json, "Mode");
    std::optional<int8_t> amount = ExtractInt8(json, "Amount");
    VerifyOrReturnError(mode.has_value() && amount.has_value(), CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t buffer[32];
    TLV::TLVWriter writer;
    writer.Init(buffer, sizeof(buffer));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(1), endpointId));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(2), *mode));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(3), *amount));
    ReturnErrorOnFailure(writer.EndContainer(outerType));
    ReturnErrorOnFailure(writer.Finalize());

    return registry.HandleAction("HeatPumpSetpointRaiseLower"_span, ByteSpan(buffer, writer.GetLengthWritten()));
}

} // namespace chip::app::NamedPipe
