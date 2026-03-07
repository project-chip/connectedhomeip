/*
 *
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

#include <app/clusters/electrical-energy-measurement-server/ElectricalEnergyMeasurementDelegate.h>
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

/** @brief Data structure to hold measurement data for backwards compatibility */
struct MeasurementData
{
    Structs::MeasurementAccuracyStruct::Type measurementAccuracy;
    Optional<Structs::EnergyMeasurementStruct::Type> cumulativeImported;
    Optional<Structs::EnergyMeasurementStruct::Type> cumulativeExported;
    Optional<Structs::EnergyMeasurementStruct::Type> periodicImported;
    Optional<Structs::EnergyMeasurementStruct::Type> periodicExported;
    Optional<Structs::CumulativeEnergyResetStruct::Type> cumulativeReset;
};

class ElectricalEnergyMeasurementCluster : public DefaultServerCluster
{
public:
    using MeasurementAccuracyStruct   = Structs::MeasurementAccuracyStruct::Type;
    using EnergyMeasurementStruct     = Structs::EnergyMeasurementStruct::Type;
    using CumulativeEnergyResetStruct = Structs::CumulativeEnergyResetStruct::Type;

    using OptionalAttributesSet = OptionalAttributeSet< //
        Attributes::CumulativeEnergyImported::Id,       //
        Attributes::CumulativeEnergyExported::Id,       //
        Attributes::PeriodicEnergyImported::Id,         //
        Attributes::PeriodicEnergyExported::Id,         //
        Attributes::CumulativeEnergyReset::Id           //
        >;

    static constexpr System::Clock::Timeout kMaxReportInterval = System::Clock::Seconds32(60);
    static constexpr System::Clock::Timeout kMinReportInterval = System::Clock::Seconds32(1);

    struct Config
    {
        EndpointId endpointId;
        BitMask<Feature> featureFlags;
        OptionalAttributesSet optionalAttributes;
        const Structs::MeasurementAccuracyStruct::Type & accuracyStruct;
        Delegate & delegate;
        TimerDelegate & timerDelegate;
        System::Clock::Timeout reportInterval = kMaxReportInterval;
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

    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown(ClusterShutdownType shutdownType) override;

    const OptionalAttributesSet & OptionalAttributes() const { return mEnabledOptionalAttributes; }
    const BitFlags<Feature> & Features() const { return mFeatureFlags; }

    /** @brief Direct access to measurement data - for backwards compatibility */
    const MeasurementData * GetMeasurementData() const { return &mMeasurementData; }

    /** @brief Getters */
    void GetMeasurementAccuracy(MeasurementAccuracyStruct & outValue) const;
    CHIP_ERROR GetCumulativeEnergyImported(Optional<EnergyMeasurementStruct> & outValue) const;
    CHIP_ERROR GetCumulativeEnergyExported(Optional<EnergyMeasurementStruct> & outValue) const;
    CHIP_ERROR GetPeriodicEnergyImported(Optional<EnergyMeasurementStruct> & outValue) const;
    CHIP_ERROR GetPeriodicEnergyExported(Optional<EnergyMeasurementStruct> & outValue) const;
    CHIP_ERROR GetCumulativeEnergyReset(Optional<CumulativeEnergyResetStruct> & outValue) const;

    /** @brief Sets the measurement accuracy.
     *
     * @return CHIP_ERROR_NO_MEMORY if the deep copy of accuracyRanges fails to allocate.
     *
     * @note Use the constructor with Config::accuracyStruct for zero-copy reference to long-lived (e.g. flash) data.
     */
    CHIP_ERROR SetMeasurementAccuracy(const MeasurementAccuracyStruct & value);
    CHIP_ERROR SetCumulativeEnergyReset(const Optional<CumulativeEnergyResetStruct> & value);

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    /** @brief Handle to Force an immediate report:
     *  pulls readings from the delegate, updates attributes, generates events, and resets the periodic timer.
     *
     * @note Enforces the min 1s reporting rule, if a report was generated less than 1s ago, this will do nothing.
     */
    void GenerateReport();

    /** @brief Legacy snapshot methods -- kept for backwards compatibility with CodegenIntegration */
    void CumulativeEnergySnapshot(const Optional<EnergyMeasurementStruct> & energyImported,
                                  const Optional<EnergyMeasurementStruct> & energyExported);
    void PeriodicEnergySnapshot(const Optional<EnergyMeasurementStruct> & energyImported,
                                const Optional<EnergyMeasurementStruct> & energyExported);

private:
    CHIP_ERROR SetCumulativeEnergyImported(const Optional<EnergyMeasurementStruct> & value);
    CHIP_ERROR SetCumulativeEnergyExported(const Optional<EnergyMeasurementStruct> & value);
    CHIP_ERROR SetPeriodicEnergyImported(const Optional<EnergyMeasurementStruct> & value);
    CHIP_ERROR SetPeriodicEnergyExported(const Optional<EnergyMeasurementStruct> & value);

    /** @brief Builds an EnergyMeasurementStruct with timestamps, carrying forward the previous endTimestamp as the new
     * startTimestamp. */
    Optional<EnergyMeasurementStruct> BuildMeasurement(DataModel::Nullable<int64_t> energy,
                                                       const Optional<EnergyMeasurementStruct> & previous);

    /** @brief Core report logic shared by GenerateReport() and the timer callback */
    void DoGenerateReport();

    void StartReportTimer();
    void CancelReportTimer();

    class ReportTimer : public TimerContext
    {
    public:
        ReportTimer(ElectricalEnergyMeasurementCluster & cluster) : mCluster(cluster) {}
        void TimerFired() override;

    private:
        ElectricalEnergyMeasurementCluster & mCluster;
    };

    const BitFlags<Feature> mFeatureFlags;
    const OptionalAttributesSet mEnabledOptionalAttributes;
    MeasurementData mMeasurementData;

    /** @brief Owns the accuracyRanges backing store; either references long-lived data (ReferenceExisting) when passing in
     * Config::accuracyStruct or an allocated copy when using the SetMeasurementAccuracy() method */
    ReadOnlyBuffer<Structs::MeasurementAccuracyRangeStruct::Type> mAccuracyRangesStorage;

    Delegate & mDelegate;
    TimerDelegate & mTimerDelegate;
    System::Clock::Timeout mReportInterval;
    System::Clock::Timestamp mLastReportTime{ System::Clock::kZero };
    ReportTimer mReportTimer{ *this };
};

} // namespace ElectricalEnergyMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
