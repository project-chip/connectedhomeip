/*
 *
 *    Copyright (c) 2025-2026 Project CHIP Authors
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

#include <app/cluster-building-blocks/QuieterReporting.h>
#include <app/clusters/electrical-energy-measurement-server/ElectricalEnergyMeasurementDelegate.h>
#include <app/data-model/Nullable.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/ElectricalEnergyMeasurement/AttributeIds.h>
#include <clusters/ElectricalEnergyMeasurement/ClusterId.h>
#include <clusters/ElectricalEnergyMeasurement/Structs.h>
#include <lib/core/Optional.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <lib/support/TimerDelegate.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalEnergyMeasurement {

class ElectricalEnergyMeasurementCluster : public DefaultServerCluster
{
public:
    using MeasurementAccuracyStruct   = Structs::MeasurementAccuracyStruct::Type;
    using EnergyMeasurementStruct     = Structs::EnergyMeasurementStruct::Type;
    using CumulativeEnergyResetStruct = Structs::CumulativeEnergyResetStruct::Type;
    template <typename T>
    using Nullable = DataModel::Nullable<T>;

    using OptionalAttributesSet = OptionalAttributeSet< //
        Attributes::CumulativeEnergyImported::Id,       //
        Attributes::CumulativeEnergyExported::Id,       //
        Attributes::PeriodicEnergyImported::Id,         //
        Attributes::PeriodicEnergyExported::Id,         //
        Attributes::CumulativeEnergyReset::Id           //
        >;

    //
    // Per Spec, a server cannot report more frequently than every 1 second:
    // 6.X. [Cumulative/Periodic]Energy[Imported/Exported] Attribute:
    // The server SHALL NOT mark this attribute ready for report if the last time this was done was more recently than 1 second ago.
    //
    static constexpr System::Clock::Timeout kMinReportInterval = System::Clock::Seconds32(1);

    /**
     * Quieter reporting wrapper for Nullable<EnergyMeasurementStruct>.
     *
     * QuieterReportingAttribute<T> only accepts arithmetic T. This wrapper provides
     * the same semantics for the EnergyMeasurementStruct and updates the dirty state based on
     * the struct's `energy` field via an internal QuieterReportingAttribute<int64_t>.
     *
     */
    class QuieterReportingEnergyAttribute
    {
    public:
        using StructType = Structs::EnergyMeasurementStruct::Type;
        template <typename T>
        using Nullable = DataModel::Nullable<T>;

        QuieterReportingEnergyAttribute() = default;

        explicit QuieterReportingEnergyAttribute(const Nullable<StructType> & initialValue) :
            mValue(initialValue), mEnergyTracker(EnergyKey(initialValue))
        {}

        const Nullable<StructType> & value() const { return mValue; }

        /**
         * Update the held struct and compute reporting state based on the energy field.
         *
         * @param newValue new struct value (or null) to store
         * @param now      monotonic timestamp at the time of the call
         * @return AttributeDirtyState::kMustReport if a report is required, otherwise kNoReportNeeded.
         */
        AttributeDirtyState SetValue(const Nullable<StructType> & newValue, System::Clock::Milliseconds64 now)
        {

            auto predicate            = mEnergyTracker.GetPredicateForSufficientTimeSinceLastDirty(kMinReportInterval);
            AttributeDirtyState state = mEnergyTracker.SetValue(EnergyKey(newValue), now, predicate);
            mValue                    = newValue;
            return state;
        }

    private:
        static Nullable<int64_t> EnergyKey(const Nullable<StructType> & v)
        {
            return v.IsNull() ? Nullable<int64_t>() : DataModel::MakeNullable(v.Value().energy);
        }

        Nullable<StructType> mValue;
        QuieterReportingAttribute<int64_t> mEnergyTracker;
    };
    struct Config
    {
        EndpointId endpointId;
        BitMask<Feature> featureFlags;
        OptionalAttributesSet optionalAttributes;
        const Structs::MeasurementAccuracyStruct::Type & accuracyStruct;
        Delegate & delegate;
        TimerDelegate & timerDelegate;
    };

    /** @brief Constructor for ElectricalEnergyMeasurementCluster.
     *
     * @param config The configuration for the cluster. The only optional attribute is Cumulative Energy Reset,
     *  the other attributes will be enabled based on the feature flags. Cumulative Energy Reset is only enabled
     *  if the feature flag for Cumulative Energy is enabled AND the optional attribute is set in the config.
     *
     * @note The accuracyStruct must outlive the cluster to avoid dangling pointers.
     */
    ElectricalEnergyMeasurementCluster(const Config & config);

    void Shutdown(ClusterShutdownType shutdownType) override;

    const OptionalAttributesSet & OptionalAttributes() const { return mEnabledOptionalAttributes; }
    const BitFlags<Feature> & Features() const { return mFeatureFlags; }

    /** @brief Getters */
    void GetMeasurementAccuracy(MeasurementAccuracyStruct & outValue) const;
    CHIP_ERROR GetCumulativeEnergyImported(Nullable<EnergyMeasurementStruct> & outValue) const;
    CHIP_ERROR GetCumulativeEnergyExported(Nullable<EnergyMeasurementStruct> & outValue) const;
    CHIP_ERROR GetPeriodicEnergyImported(Nullable<EnergyMeasurementStruct> & outValue) const;
    CHIP_ERROR GetPeriodicEnergyExported(Nullable<EnergyMeasurementStruct> & outValue) const;
    CHIP_ERROR GetCumulativeEnergyReset(Nullable<CumulativeEnergyResetStruct> & outValue) const;

    /** @brief Sets the measurement accuracy.
     *
     * @return CHIP_ERROR_NO_MEMORY if the deep copy of accuracyRanges fails to allocate.
     *
     * @note Use the constructor with Config::accuracyStruct for zero-copy reference to long-lived (e.g. flash) data.
     */
    CHIP_ERROR SetMeasurementAccuracy(const MeasurementAccuracyStruct & value);
    CHIP_ERROR SetCumulativeEnergyReset(const Nullable<CumulativeEnergyResetStruct> & value);

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    /** @brief Handle to request an immediate snapshot/report:
     *  pulls readings from the delegate, updates attributes, and generates events.
     *
     * @note Enforces the minimum reporting interval: if a report was generated less than kMinReportInterval ago, this will
     *        update the cluster values without generating a report.
     */
    void GenerateSnapshots();

    /** @brief Legacy snapshot methods -- kept for backwards compatibility with CodegenIntegration */
    void CumulativeEnergySnapshot(const Nullable<EnergyMeasurementStruct> & energyImported,
                                  const Nullable<EnergyMeasurementStruct> & energyExported);
    void PeriodicEnergySnapshot(const Nullable<EnergyMeasurementStruct> & energyImported,
                                const Nullable<EnergyMeasurementStruct> & energyExported);

private:
    AttributeDirtyState SetCumulativeEnergyImported(const Nullable<EnergyMeasurementStruct> & value);
    AttributeDirtyState SetCumulativeEnergyExported(const Nullable<EnergyMeasurementStruct> & value);
    AttributeDirtyState SetPeriodicEnergyImported(const Nullable<EnergyMeasurementStruct> & value);
    AttributeDirtyState SetPeriodicEnergyExported(const Nullable<EnergyMeasurementStruct> & value);

    // Attributes
    Structs::MeasurementAccuracyStruct::Type mMeasurementAccuracy;
    QuieterReportingEnergyAttribute mCumulativeImported;
    QuieterReportingEnergyAttribute mCumulativeExported;
    QuieterReportingEnergyAttribute mPeriodicImported;
    QuieterReportingEnergyAttribute mPeriodicExported;
    Nullable<Structs::CumulativeEnergyResetStruct::Type> mCumulativeReset;

    /** @brief Builds an EnergyMeasurementStruct with timestamps. For periodic (isCumulative == false), copies previous
     *  endTimestamp/endSystime into start*. For cumulative (isCumulative == true), start fields stay unset (spec). */
    DataModel::Nullable<EnergyMeasurementStruct>
    BuildMeasurement(Nullable<int64_t> energy, const Nullable<EnergyMeasurementStruct> & previous, bool isCumulative);

    const BitFlags<Feature> mFeatureFlags;
    const OptionalAttributesSet mEnabledOptionalAttributes;

    /** @brief Owns the accuracyRanges backing store; either references long-lived data (ReferenceExisting) when passing in
     * Config::accuracyStruct or an allocated copy when using the SetMeasurementAccuracy() method */
    ReadOnlyBuffer<Structs::MeasurementAccuracyRangeStruct::Type> mAccuracyRangesStorage;

    Delegate & mDelegate;
    TimerDelegate & mTimerDelegate;
};

} // namespace ElectricalEnergyMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
