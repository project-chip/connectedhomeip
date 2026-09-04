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

#include <app/clusters/electrical-energy-measurement-server/ElectricalEnergyMeasurementCluster.h>
#include <lib/core/DataModelTypes.h>
#include <oob-accessors/OOBAccessor.h>

namespace chip::app {

class ElectricalEnergyMeasurementOOBAccessor : public OOBAccessor
{
public:
    ElectricalEnergyMeasurementOOBAccessor(
        Clusters::ElectricalEnergyMeasurement::ElectricalEnergyMeasurementCluster & cluster, EndpointId endpointId) :
        mCluster(cluster),
        mEndpointId(endpointId)
    {}

    std::optional<CHIP_ERROR> HandleAction(CharSpan action, ByteSpan tlvData) override;

private:
    Clusters::ElectricalEnergyMeasurement::ElectricalEnergyMeasurementCluster & mCluster;
    EndpointId mEndpointId;
};

} // namespace chip::app
