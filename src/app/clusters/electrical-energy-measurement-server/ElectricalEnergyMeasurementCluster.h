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

#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/ElectricalEnergyMeasurement/AttributeIds.h>
#include <clusters/ElectricalEnergyMeasurement/ClusterId.h>
#include <clusters/ElectricalEnergyMeasurement/Structs.h>
#include <lib/core/Optional.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalEnergyMeasurement {

// Data structure to hold measurement data for backwards compatibility
struct MeasurementData
{
    Structs::MeasurementAccuracyStruct::Type measurementAccuracy;
    Optional<Structs::EnergyMeasurementStruct::Type> cumulativeImported;
    Optional<Structs::EnergyMeasurementStruct::Type> cumulativeExported;
    Optional<Structs::EnergyMeasurementStruct::Type> periodicImported;
    Optional<Structs::EnergyMeasurementStruct::Type> periodicExported;
    Optional<Structs::CumulativeEnergyResetStruct::Type> cumulativeReset;
};
}; // namespace ElectricalEnergyMeasurement

class ElectricalEnergyMeasurementCluster : public DefaultServerCluster
{
public:
    // Type aliases for shorter struct type names
    using MeasurementAccuracyStruct   = ElectricalEnergyMeasurement::Structs::MeasurementAccuracyStruct::Type;
    using EnergyMeasurementStruct     = ElectricalEnergyMeasurement::Structs::EnergyMeasurementStruct::Type;
    using CumulativeEnergyResetStruct = ElectricalEnergyMeasurement::Structs::CumulativeEnergyResetStruct::Type;

    using OptionalAttributesSet = OptionalAttributeSet<                        //
        ElectricalEnergyMeasurement::Attributes::CumulativeEnergyImported::Id, //
        ElectricalEnergyMeasurement::Attributes::CumulativeEnergyExported::Id, //
        ElectricalEnergyMeasurement::Attributes::PeriodicEnergyImported::Id,   //
        ElectricalEnergyMeasurement::Attributes::PeriodicEnergyExported::Id,   //
        ElectricalEnergyMeasurement::Attributes::CumulativeEnergyReset::Id     //
        >;

    struct Config
    {
        constexpr Config(EndpointId endpoint, BitMask<ElectricalEnergyMeasurement::Feature> featureFlags,
                         OptionalAttributesSet & optionalAttributes) :
            endpointId(endpoint), mFeatureFlags(featureFlags), mEnabledOptionalAttributes(optionalAttributes)
        {}

        EndpointId endpointId;
        const BitFlags<ElectricalEnergyMeasurement::Feature> mFeatureFlags;
        OptionalAttributesSet mEnabledOptionalAttributes;
    };

    ElectricalEnergyMeasurementCluster(const Config & config);

    OptionalAttributesSet & OptionalAttributes() { return mEnabledOptionalAttributes; }
    const OptionalAttributesSet & OptionalAttributes() const { return mEnabledOptionalAttributes; }
    const BitFlags<ElectricalEnergyMeasurement::Feature> & Features() const { return mFeatureFlags; }

    // Direct access to measurement data - for backwards compatibility
    const ElectricalEnergyMeasurement::MeasurementData * GetMeasurementData() const { return &mMeasurementData; }

    // Getters - return copies with error checking
    CHIP_ERROR GetMeasurementAccuracy(MeasurementAccuracyStruct & outValue) const;
    CHIP_ERROR GetCumulativeEnergyImported(Optional<EnergyMeasurementStruct> & outValue) const;
    CHIP_ERROR GetCumulativeEnergyExported(Optional<EnergyMeasurementStruct> & outValue) const;
    CHIP_ERROR GetPeriodicEnergyImported(Optional<EnergyMeasurementStruct> & outValue) const;
    CHIP_ERROR GetPeriodicEnergyExported(Optional<EnergyMeasurementStruct> & outValue) const;
    CHIP_ERROR GetCumulativeEnergyReset(Optional<CumulativeEnergyResetStruct> & outValue) const;

    // Setters - update values and notify data model
    CHIP_ERROR SetMeasurementAccuracy(const MeasurementAccuracyStruct & value);
    CHIP_ERROR SetCumulativeEnergyImported(const Optional<EnergyMeasurementStruct> & value);
    CHIP_ERROR SetCumulativeEnergyExported(const Optional<EnergyMeasurementStruct> & value);
    CHIP_ERROR SetPeriodicEnergyImported(const Optional<EnergyMeasurementStruct> & value);
    CHIP_ERROR SetPeriodicEnergyExported(const Optional<EnergyMeasurementStruct> & value);
    CHIP_ERROR SetCumulativeEnergyReset(const Optional<CumulativeEnergyResetStruct> & value);

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

private:
    const BitFlags<ElectricalEnergyMeasurement::Feature> mFeatureFlags;
    OptionalAttributesSet mEnabledOptionalAttributes;
    ElectricalEnergyMeasurement::MeasurementData mMeasurementData;
};

} // namespace Clusters
} // namespace app
} // namespace chip
