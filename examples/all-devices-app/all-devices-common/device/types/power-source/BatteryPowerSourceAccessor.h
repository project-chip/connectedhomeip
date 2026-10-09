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

#include <device/types/power-source/BatteryPowerSource.h>
#include <oob-accessors/OOBAccessor.h>

namespace chip::app {

class BatteryPowerSourceAccessor : public OOBAccessor
{
public:
    BatteryPowerSourceAccessor(BatteryPowerSource::SimpleBatteryPowerSourceCluster & cluster, EndpointId endpointId) :
        mCluster(cluster), mEndpointId(endpointId)
    {}

    explicit BatteryPowerSourceAccessor(BatteryPowerSource & device) :
        BatteryPowerSourceAccessor(device.BatteryPowerSourceCluster(), device.GetEndpointId())
    {}

    ~BatteryPowerSourceAccessor() override = default;

    std::optional<CHIP_ERROR> HandleAction(CharSpan actionName, ByteSpan tlvBuffer) override;

private:
    BatteryPowerSource::SimpleBatteryPowerSourceCluster & mCluster;
    EndpointId mEndpointId;
};

} // namespace chip::app
