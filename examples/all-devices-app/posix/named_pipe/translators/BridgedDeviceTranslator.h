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

#pragma once

#include <posix/named_pipe/CommandTranslator.h>

namespace chip::app::NamedPipe {

/**
 * Named pipe usage (json):
 * AddBridgedDevice:
 * {
 *    "Name": "AddBridgedDevice"
 *    "EndpointId" | "AggregatorEndpointId": EndpointId (uint16_t)
 *        Optional. An endpoint id of an existing aggregator device.
 *        If an aggregator is found, `Bridged Node` will be created as child,
 *        and the new device will be added as a child of the latter device.
 *
 *    the `Aggregator`
 *           |
 *  a new `Bridged Node`
 *           |
 *     the new device
 *
 *        If not present,  or `kInvalidEndpointId` is specified, a default aggregator device will be used.
 *        Otherwise if no aggregator is found on the specified endpoint, the command will fail.
 *
 *    "Device": String
 *        The device type name, e.g. "electrical-sensor". See `examples/all-devices-app/README.md` for the list of supported device
 * types.
 * }
 *
 * RemoveBridgedDevice:
 * {
 *    "Name": String
 *        Must be "RemoveBridgedDevice"
 *    "DeviceInterfaceId" : BridgedDeviceManager::DeviceInterfaceId (uint16_t)
 *        The deviceInterfaceId of the bridged device to be removed.
 * }
 *
 * Examples:
 *  echo '{"Name": "AddBridgedDevice", "AggregatorEndpointId": 5, "Device": "electrical-sensor"}'> /tmp/acs_fifo
 *  echo '{"Name": "RemoveBridgedDevice", "DeviceInterfaceId": 2}'> /tmp/acs_fifo
 */

class BridgedDeviceTranslator : public CommandTranslator
{
public:
    static Span<const CharSpan> GetActionNames()
    {
        static constexpr CharSpan kNames[] = { "AddBridgedDevice"_span, "RemoveBridgedDevice"_span };
        return Span<const CharSpan>(kNames);
    }

    CHIP_ERROR TranslateAndExecute(EndpointId endpointId, const Json::Value & json, OOBAccessorRegistry & registry) const override;
};

} // namespace chip::app::NamedPipe
