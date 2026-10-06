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

        // By default if "EndpointId" not specified, TranslateAndExecute will provide `kRootEndpointId`,
        // which is not the behaviour needed here.
        endpointId = ExtractUInt<EndpointId>(json, "EndpointId").value_or(kInvalidEndpointId);
        endpointId = ExtractUInt<EndpointId>(json, "AggregatorEndpointId").value_or(endpointId);

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
        return DispatchAction(registry, "RemoveBridgedDevice"_span, endpointId, deviceInterfaceId.value());
    }
    return CHIP_ERROR_NOT_FOUND;
}

} // namespace chip::app::NamedPipe
