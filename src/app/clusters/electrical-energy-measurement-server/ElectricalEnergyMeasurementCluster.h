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

    static constexpr System::Clock::Timeout kMaxReportDelayInterval = System::Clock::Seconds32(60);
    /// Alias for the maximum delay between timer-driven snapshot attempts (same as kMaxReportDelayInterval).
    static constexpr System::Clock::Timeout kMaxReportInterval = kMaxReportDelayInterval;
    static constexpr System::Clock::Timeout kMinReportInterval = System::Clock::Seconds32(1);

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

    /** @brief Handle to Force an immediate report:
     *  pulls readings from the delegate, updates attributes, generates events, and resets the snapshot timer.
     *
     * @note Enforces the min 1s reporting rule, if a report was generated less than 1s ago, this will do nothing and the
     * snapshot timer will ensure the next snapshot is generated within 60 seconds of ignoring the attribute changes.
     */
    void GenerateSnapshots();

    /** @brief Legacy snapshot methods -- kept for backwards compatibility with CodegenIntegration */
    void CumulativeEnergySnapshot(const Nullable<EnergyMeasurementStruct> & energyImported,
                                  const Nullable<EnergyMeasurementStruct> & energyExported);
    void PeriodicEnergySnapshot(const Nullable<EnergyMeasurementStruct> & energyImported,
                                const Nullable<EnergyMeasurementStruct> & energyExported);

private:
    enum class AttributeIgnoredDirtyState : uint32_t
    {
        CumulativeEnergyImported = 0x1,
        CumulativeEnergyExported = 0x2,
        PeriodicEnergyImported   = 0x4,
        PeriodicEnergyExported   = 0x8,
    };

    CHIP_ERROR SetCumulativeEnergyImported(const Nullable<EnergyMeasurementStruct> & value);
    CHIP_ERROR SetCumulativeEnergyExported(const Nullable<EnergyMeasurementStruct> & value);
    CHIP_ERROR SetPeriodicEnergyImported(const Nullable<EnergyMeasurementStruct> & value);
    CHIP_ERROR SetPeriodicEnergyExported(const Nullable<EnergyMeasurementStruct> & value);

    // Attributes
    Structs::MeasurementAccuracyStruct::Type mMeasurementAccuracy;
    Nullable<Structs::EnergyMeasurementStruct::Type> mCumulativeImported;
    Nullable<Structs::EnergyMeasurementStruct::Type> mCumulativeExported;
    Nullable<Structs::EnergyMeasurementStruct::Type> mPeriodicImported;
    Nullable<Structs::EnergyMeasurementStruct::Type> mPeriodicExported;
    Nullable<Structs::CumulativeEnergyResetStruct::Type> mCumulativeReset;

    /** @brief Builds an EnergyMeasurementStruct with timestamps. For periodic (isCumulative == false), copies previous
     *  endTimestamp/endSystime into start*. For cumulative (isCumulative == true), start fields stay unset (spec). */
    DataModel::Nullable<EnergyMeasurementStruct>
    BuildMeasurement(Nullable<int64_t> energy, const Nullable<EnergyMeasurementStruct> & previous, bool isCumulative);

    void StartReportDelayTimer();
    void CancelReportDelayTimer();

    class ReportDelayTimer : public TimerContext
    {
    public:
        ReportDelayTimer(ElectricalEnergyMeasurementCluster & cluster) : mCluster(cluster) {}
        void TimerFired() override;

    private:
        ElectricalEnergyMeasurementCluster & mCluster;
    };

    const BitFlags<Feature> mFeatureFlags;
    const OptionalAttributesSet mEnabledOptionalAttributes;

    /** @brief Owns the accuracyRanges backing store; either references long-lived data (ReferenceExisting) when passing in
     * Config::accuracyStruct or an allocated copy when using the SetMeasurementAccuracy() method */
    ReadOnlyBuffer<Structs::MeasurementAccuracyRangeStruct::Type> mAccuracyRangesStorage;

    Delegate & mDelegate;
    TimerDelegate & mTimerDelegate;

    BitFlags<AttributeIgnoredDirtyState> mAttributeDirtyState;
    ReportDelayTimer mReportDelayTimer{ *this };
};

} // namespace ElectricalEnergyMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
