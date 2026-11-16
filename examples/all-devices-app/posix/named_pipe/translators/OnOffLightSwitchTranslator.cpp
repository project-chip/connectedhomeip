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

#include <posix/named_pipe/translators/OnOffLightSwitchTranslator.h>

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

    return registry.HandleAction("SwitchTriggerEffect"_span, ByteSpan(buffer, writer.GetLengthWritten()));
}

} // namespace

CHIP_ERROR OnOffLightSwitchTranslator::TranslateAndExecute(EndpointId endpointId, const Json::Value & json,
                                                           OOBAccessorRegistry & registry) const
{
    if (!json.isObject() || !json.isMember("Name") || !json["Name"].isString())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    std::string action = json["Name"].asString();
    if (action == "SwitchOn")
    {
        return DispatchAction(registry, "SwitchOn"_span, endpointId);
    }
    if (action == "SwitchOff")
    {
        return DispatchAction(registry, "SwitchOff"_span, endpointId);
    }
    if (action == "SwitchToggle")
    {
        return DispatchAction(registry, "SwitchToggle"_span, endpointId);
    }
    if (action == "SwitchIdentify")
    {
        auto identifyTime = ExtractUInt<uint16_t>(json, "IdentifyTime");
        VerifyOrReturnError(identifyTime.has_value(), CHIP_ERROR_INVALID_ARGUMENT);
        return DispatchAction(registry, "SwitchIdentify"_span, endpointId, *identifyTime);
    }
    if (action == "SwitchTriggerEffect")
    {
        auto effectIdentifier = ExtractUInt<uint8_t>(json, "EffectIdentifier");
        auto effectVariant    = json.isMember("EffectVariant") ? ExtractUInt<uint8_t>(json, "EffectVariant")
                                                               : std::make_optional(kDefaultEffectVariant);
        VerifyOrReturnError(effectIdentifier.has_value() && effectVariant.has_value(), CHIP_ERROR_INVALID_ARGUMENT);
        return DispatchTriggerEffect(registry, endpointId, *effectIdentifier, *effectVariant);
    }
    return CHIP_ERROR_NOT_FOUND;
}

} // namespace chip::app::NamedPipe
