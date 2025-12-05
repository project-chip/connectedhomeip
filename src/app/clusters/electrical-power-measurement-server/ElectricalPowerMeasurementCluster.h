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

#include <app/clusters/electrical-power-measurement-server/ElectricalPowerMeasurementDelegate.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/ElectricalPowerMeasurement/AttributeIds.h>
#include <clusters/ElectricalPowerMeasurement/ClusterId.h>
#include <clusters/ElectricalPowerMeasurement/Metadata.h>
// #include <clusters/ElectricalPowerMeasurement/Structs.h>
// #include <lib/core/Optional.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalPowerMeasurement {

class ElectricalPowerMeasurementCluster : public DefaultServerCluster
{
public:
    using OptionalAttributesSet = OptionalAttributeSet<               //
        ElectricalPowerMeasurement::Attributes::Ranges::Id,           // Optional
        ElectricalPowerMeasurement::Attributes::Voltage::Id,          // Optional
        ElectricalPowerMeasurement::Attributes::ActiveCurrent::Id,    // Optional
        ElectricalPowerMeasurement::Attributes::ReactiveCurrent::Id,  // Optional but depends on AlternateCurrent feature
        ElectricalPowerMeasurement::Attributes::ApparentCurrent::Id,  // Optional but depends on AlternateCurrent feature
        ElectricalPowerMeasurement::Attributes::ReactivePower::Id,    // Optional but depends on AlternateCurrent feature
        ElectricalPowerMeasurement::Attributes::ApparentPower::Id,    // Optional but depends on AlternateCurrent feature
        ElectricalPowerMeasurement::Attributes::RMSVoltage::Id,       // Optional but depends on AlternateCurrent feature
        ElectricalPowerMeasurement::Attributes::RMSCurrent::Id,       // Optional but depends on AlternateCurrent feature
        ElectricalPowerMeasurement::Attributes::RMSPower::Id,         // Optional but depends on AlternateCurrent feature
        ElectricalPowerMeasurement::Attributes::Frequency::Id,        // Optional but depends on AlternateCurrent feature
        ElectricalPowerMeasurement::Attributes::HarmonicCurrents::Id, // Derived from Harmonics feature
        ElectricalPowerMeasurement::Attributes::HarmonicPhases::Id,   // Derived from PowerQuality feature
        ElectricalPowerMeasurement::Attributes::PowerFactor::Id,      // Optional but depends on AlternateCurrent feature
        ElectricalPowerMeasurement::Attributes::NeutralCurrent::Id    // Optional but depends on PolyphasePower feature
        >;

    struct Config
    {
        EndpointId endpointId;
        Delegate & delegate;
        BitMask<Feature> features;
        OptionalAttributesSet optionalAttributes;
    };

    ElectricalPowerMeasurementCluster(const Config & config) :
        DefaultServerCluster({ config.endpointId, ElectricalPowerMeasurement::Id }), mDelegate(config.delegate),
        mFeatureFlags(config.features), mEnabledOptionalAttributes([&]() {
            OptionalAttributesSet attrs = config.optionalAttributes;
            // Set attributes based on conformance of feature flags
            attrs.Set<Attributes::HarmonicCurrents::Id>(config.features.Has(Feature::kHarmonics));
            attrs.Set<Attributes::HarmonicPhases::Id>(config.features.Has(Feature::kPowerQuality));
            return attrs;
        }())
    {
        mDelegate.SetEndpointId(config.endpointId);
    }

    CHIP_ERROR Startup(ServerClusterContext & context) override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

private:
    Delegate & mDelegate;
    const BitMask<Feature> mFeatureFlags;
    const OptionalAttributesSet mEnabledOptionalAttributes;

    CHIP_ERROR EncodeAccuracy(const AttributeValueEncoder::ListEncodeHelper & aEncoder);
    CHIP_ERROR EncodeRanges(const AttributeValueEncoder::ListEncodeHelper & aEncoder);
    CHIP_ERROR EncodeHarmonicCurrents(const AttributeValueEncoder::ListEncodeHelper & aEncoder);
    CHIP_ERROR EncodeHarmonicPhases(const AttributeValueEncoder::ListEncodeHelper & aEncoder);
};

} // namespace ElectricalPowerMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
