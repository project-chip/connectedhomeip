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

enum class OptionalAttributes : uint32_t
{
    kOptionalAttributeCumulativeEnergyReset = 0x1,
};
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
        EndpointId endpointId;
        BitMask<ElectricalEnergyMeasurement::Feature> featureFlags;
        BitMask<ElectricalEnergyMeasurement::OptionalAttributes> optionalAttributes;
    };

    ElectricalEnergyMeasurementCluster(const Config & config) :
        DefaultServerCluster({ config.endpointId, ElectricalEnergyMeasurement::Id }), mFeatureFlags(config.featureFlags),
        mEnabledOptionalAttributes([&]() {
            OptionalAttributesSet attrs;
            attrs.Set<ElectricalEnergyMeasurement::Attributes::CumulativeEnergyImported::Id>(config.featureFlags.HasAll(
                ElectricalEnergyMeasurement::Feature::kCumulativeEnergy, ElectricalEnergyMeasurement::Feature::kImportedEnergy));
            attrs.Set<ElectricalEnergyMeasurement::Attributes::CumulativeEnergyExported::Id>((config.featureFlags.HasAll(
                ElectricalEnergyMeasurement::Feature::kCumulativeEnergy, ElectricalEnergyMeasurement::Feature::kExportedEnergy)));
            attrs.Set<ElectricalEnergyMeasurement::Attributes::PeriodicEnergyImported::Id>(config.featureFlags.HasAll(
                ElectricalEnergyMeasurement::Feature::kPeriodicEnergy, ElectricalEnergyMeasurement::Feature::kImportedEnergy));
            attrs.Set<ElectricalEnergyMeasurement::Attributes::PeriodicEnergyExported::Id>(config.featureFlags.HasAll(
                ElectricalEnergyMeasurement::Feature::kPeriodicEnergy, ElectricalEnergyMeasurement::Feature::kExportedEnergy));
            attrs.Set<ElectricalEnergyMeasurement::Attributes::CumulativeEnergyReset::Id>(
                config.optionalAttributes.Has(
                    ElectricalEnergyMeasurement::OptionalAttributes::kOptionalAttributeCumulativeEnergyReset) &&
                config.featureFlags.Has(ElectricalEnergyMeasurement::Feature::kCumulativeEnergy));
            return attrs;
        }())
    {}

    const OptionalAttributesSet & OptionalAttributes() const { return mEnabledOptionalAttributes; }
    const BitFlags<ElectricalEnergyMeasurement::Feature> & Features() const { return mFeatureFlags; }

    // Direct access to measurement data - for backwards compatibility
    const ElectricalEnergyMeasurement::MeasurementData * GetMeasurementData() const { return &mMeasurementData; }

    // Getters - return copies with error checking
    void GetMeasurementAccuracy(MeasurementAccuracyStruct & outValue) const;
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

    void CumulativeEnergySnapshot(const Optional<EnergyMeasurementStruct> & energyImported,
                                  const Optional<EnergyMeasurementStruct> & energyExported);

    void PeriodicEnergySnapshot(const Optional<EnergyMeasurementStruct> & energyImported,
                                const Optional<EnergyMeasurementStruct> & energyExported);

private:
    const BitFlags<ElectricalEnergyMeasurement::Feature> mFeatureFlags;
    const OptionalAttributesSet mEnabledOptionalAttributes;
    ElectricalEnergyMeasurement::MeasurementData mMeasurementData;
};

} // namespace Clusters
} // namespace app
} // namespace chip
