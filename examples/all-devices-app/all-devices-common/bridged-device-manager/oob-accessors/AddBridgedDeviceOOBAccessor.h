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

#include <bridged-device-manager/BridgedDeviceManager.h>
#include <lib/core/TLV.h>
#include <lib/support/logging/CHIPLogging.h>
#include <oob-accessors/OOBAccessor.h>

#include <optional>
#include <string>
#include <vector>

namespace chip::app {

/**
 * Usage:
 *    Tag(1): EndpointId aggregatorEndpointId
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
 *    Tag(2): String device
 *        The device type name, e.g. "electrical-sensor". See `examples/all-devices-app/README.md` for the list of supported device
 * types.
 */

class AddBridgedDeviceOOBAccessor : public OOBAccessor
{
public:
    explicit AddBridgedDeviceOOBAccessor(BridgedDeviceManager & bridgedDeviceManager);
    std::optional<CHIP_ERROR> HandleAction(CharSpan action, ByteSpan tlvData) override;

private:
    BridgedDeviceManager & mBridgedDeviceManager;
};

} // namespace chip::app
