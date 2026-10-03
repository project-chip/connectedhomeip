/*
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#include <device/types/proximity-ranger/impl/LoggingProximityRanger.h>
#include <lib/core/DataModelTypes.h>
#include <oob-accessors/OOBAccessor.h>
#include <oob-accessors/OOBAccessorRegistry.h>

namespace chip {
namespace app {

// Out-of-band (OOB) accessor for the example Proximity Ranger app
class ProximityRangerOOBAccessor : public OOBAccessor
{
public:
    static constexpr uint8_t kTagEndpointId  = 1;
    static constexpr uint8_t kTagConstraints = 2;

    ProximityRangerOOBAccessor(LoggingProximityRanger & device, EndpointId endpointId) : mDevice(device), mEndpointId(endpointId) {}

    std::optional<CHIP_ERROR> HandleAction(CharSpan action, ByteSpan tlvData) override;

private:
    // Allows OOB configuration of Rnnging Contraints
    std::optional<CHIP_ERROR> HandleSetRangingConstraints(ByteSpan tlvData) const;

    LoggingProximityRanger & mDevice;
    EndpointId mEndpointId;
};

void RegisterOOBAccessors(LoggingProximityRanger & device, OOBAccessorRegistry & registry);

} // namespace app
} // namespace chip
