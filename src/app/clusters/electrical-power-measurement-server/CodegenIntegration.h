/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/electrical-power-measurement-server/ElectricalPowerMeasurementCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalPowerMeasurement {

enum class OptionalAttributes : uint32_t
{
    kOptionalAttributeRanges          = 0x1,
    kOptionalAttributeVoltage         = 0x2,
    kOptionalAttributeActiveCurrent   = 0x4,
    kOptionalAttributeReactiveCurrent = 0x8,
    kOptionalAttributeApparentCurrent = 0x10,
    kOptionalAttributeReactivePower   = 0x20,
    kOptionalAttributeApparentPower   = 0x40,
    kOptionalAttributeRMSVoltage      = 0x80,
    kOptionalAttributeRMSCurrent      = 0x100,
    kOptionalAttributeRMSPower        = 0x200,
    kOptionalAttributeFrequency       = 0x400,
    kOptionalAttributePowerFactor     = 0x800,
    kOptionalAttributeNeutralCurrent  = 0x1000,
};

class Instance
{
public:
    Instance(EndpointId aEndpointId, Delegate & aDelegate, BitMask<Feature> aFeature,
             BitMask<OptionalAttributes> aOptionalAttributes) :
        mCluster(ElectricalPowerMeasurementCluster::Config{
            .endpointId         = aEndpointId,
            .delegate           = aDelegate,
            .features           = aFeature,
            .optionalAttributes = FromLegacyOptionalAttributes(aOptionalAttributes),
        })
    {}

    CHIP_ERROR Init();
    void Shutdown();

    ~Instance() {}

private:
    static ElectricalPowerMeasurementCluster::OptionalAttributesSet
    FromLegacyOptionalAttributes(BitMask<OptionalAttributes> aOptionalAttributes);

    RegisteredServerCluster<ElectricalPowerMeasurementCluster> mCluster;
};

} // namespace ElectricalPowerMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
