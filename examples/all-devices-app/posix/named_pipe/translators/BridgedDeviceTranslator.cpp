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

#include <posix/named_pipe/translators/BridgedDeviceTranslator.h>

namespace chip::app::NamedPipe {

CHIP_ERROR BridgedDeviceTranslator::TranslateAndExecute(EndpointId endpointId, const Json::Value & json,
                                                        OOBAccessorRegistry & registry) const
{
    std::string action = json["Name"].asString();
    if (action == "AddBridgedDevice")
    {
        if (!json.isMember("Device") || !json["Device"].isString())
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        // Do not use "EndpointId", just accept "AggregatorEndpointId" for meaning clarity
        if (json.isMember("AggregatorEndpointId"))
        {
            auto aggregatorEndpointId = ExtractUInt<EndpointId>(json, "AggregatorEndpointId");
            VerifyOrReturnError(aggregatorEndpointId.has_value(), CHIP_ERROR_INVALID_ARGUMENT);
            endpointId = aggregatorEndpointId.value();
        }
        else
        {
            endpointId = kInvalidEndpointId;
        }

        const std::string device = json["Device"].asString();
        return DispatchStringAction(registry, "AddBridgedDevice"_span, endpointId, CharSpan(device.data(), device.size()));
    }
    if (action == "RemoveBridgedDevice")
    {
        auto deviceInterfaceId = ExtractUInt<uint16_t>(json, "DeviceInterfaceId");
        if (!deviceInterfaceId.has_value())
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        // Manually encode TLV data.
        // DispatchAction() will encode endpointId with Tag(1), but we do not use it, so we don't want it to even be encoded.
        uint8_t buffer[64];
        TLV::TLVWriter writer;
        writer.Init(buffer, sizeof(buffer));

        TLV::TLVType outerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(1), deviceInterfaceId.value()));
        ReturnErrorOnFailure(writer.EndContainer(outerType));
        ReturnErrorOnFailure(writer.Finalize());

        return registry.HandleAction("RemoveBridgedDevice"_span, ByteSpan(buffer, writer.GetLengthWritten()));
    }
    return CHIP_ERROR_NOT_FOUND;
}

} // namespace chip::app::NamedPipe
