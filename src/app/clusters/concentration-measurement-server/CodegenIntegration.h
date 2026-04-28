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

#include "ConcentrationMeasurementCluster.h"
#include "concentration-measurement-cluster-objects.h"
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <type_traits>

namespace chip {
namespace app {
namespace Clusters {
namespace ConcentrationMeasurement {

/**
 * Compatibility shim over ConcentrationMeasurementCluster that preserves the
 * original template-based API.
 *
 * Migration notes:
 *  - Replace Init() with registry.Register(instance.Registration()) in your
 *    ApplicationInit() or equivalent.
 *  - Init() is kept as a no-op so existing call sites compile without changes.
 *  - All Set*() methods forward directly to the underlying cluster.
 *
 * @tparam NumericMeasurementEnabled  Enables MeasuredValue, Min/Max, Uncertainty, Unit.
 * @tparam LevelIndicationEnabled     Enables LevelValue.
 * @tparam MediumLevelEnabled         Sub-flag of LevelIndication (kMedium level).
 * @tparam CriticalLevelEnabled       Sub-flag of LevelIndication (kCritical level).
 * @tparam PeakMeasurementEnabled     Enables PeakMeasuredValue / Window.
 * @tparam AverageMeasurementEnabled  Enables AverageMeasuredValue / Window.
 */
template <bool NumericMeasurementEnabled, bool LevelIndicationEnabled, bool MediumLevelEnabled, bool CriticalLevelEnabled,
          bool PeakMeasurementEnabled, bool AverageMeasurementEnabled>
class Instance
{
    static_assert(NumericMeasurementEnabled || LevelIndicationEnabled,
                  "At least one of NumericMeasurement or LevelIndication must be enabled");
    static_assert(!MediumLevelEnabled || LevelIndicationEnabled, "MediumLevelEnabled requires LevelIndicationEnabled");
    static_assert(!CriticalLevelEnabled || LevelIndicationEnabled, "CriticalLevelEnabled requires LevelIndicationEnabled");
    static_assert(!PeakMeasurementEnabled || NumericMeasurementEnabled,
                  "PeakMeasurementEnabled requires NumericMeasurementEnabled");
    static_assert(!AverageMeasurementEnabled || NumericMeasurementEnabled,
                  "AverageMeasurementEnabled requires NumericMeasurementEnabled");

public:
    // Constructor for clusters without kNumericMeasurement (level-indication only).
    Instance(EndpointId aEndpointId, ClusterId aClusterId, MeasurementMediumEnum aMeasurementMedium) :
        mCluster(aEndpointId, aClusterId, MakeFeatureFlags(), aMeasurementMedium, MeasurementUnitEnum::kUnknownEnumValue),
        mRegistration(mCluster)
    {}

    // Constructor for clusters with kNumericMeasurement.
    Instance(EndpointId aEndpointId, ClusterId aClusterId, MeasurementMediumEnum aMeasurementMedium,
             MeasurementUnitEnum aMeasurementUnit) :
        mCluster(aEndpointId, aClusterId, MakeFeatureFlags(), aMeasurementMedium, aMeasurementUnit),
        mRegistration(mCluster)
    {}

    /**
     * No-op: registration is now done via registry.Register(Registration()).
     * Kept so existing Init() call sites compile unchanged.
     */
    CHIP_ERROR Init() { return CHIP_NO_ERROR; }

    /** Pass to CodegenDataModelProvider::Instance().Registry().Register(). */
    ServerClusterRegistration & Registration() { return mRegistration; }

    // ── Numeric measurement ───────────────────────────────────────────────
    template <bool En = NumericMeasurementEnabled, typename = std::enable_if_t<En>>
    CHIP_ERROR SetMeasuredValue(DataModel::Nullable<float> v)
    {
        return mCluster.SetMeasuredValue(v);
    }

    template <bool En = NumericMeasurementEnabled, typename = std::enable_if_t<En>>
    CHIP_ERROR SetMinMeasuredValue(DataModel::Nullable<float> v)
    {
        return mCluster.SetMinMeasuredValue(v);
    }

    template <bool En = NumericMeasurementEnabled, typename = std::enable_if_t<En>>
    CHIP_ERROR SetMaxMeasuredValue(DataModel::Nullable<float> v)
    {
        return mCluster.SetMaxMeasuredValue(v);
    }

    template <bool En = NumericMeasurementEnabled, typename = std::enable_if_t<En>>
    CHIP_ERROR SetUncertainty(float v)
    {
        return mCluster.SetUncertainty(v);
    }

    template <bool En = PeakMeasurementEnabled, typename = std::enable_if_t<En>>
    CHIP_ERROR SetPeakMeasuredValue(DataModel::Nullable<float> v)
    {
        return mCluster.SetPeakMeasuredValue(v);
    }

    template <bool En = PeakMeasurementEnabled, typename = std::enable_if_t<En>>
    CHIP_ERROR SetPeakMeasuredValueWindow(uint32_t v)
    {
        return mCluster.SetPeakMeasuredValueWindow(v);
    }

    template <bool En = AverageMeasurementEnabled, typename = std::enable_if_t<En>>
    CHIP_ERROR SetAverageMeasuredValue(DataModel::Nullable<float> v)
    {
        return mCluster.SetAverageMeasuredValue(v);
    }

    template <bool En = AverageMeasurementEnabled, typename = std::enable_if_t<En>>
    CHIP_ERROR SetAverageMeasuredValueWindow(uint32_t v)
    {
        return mCluster.SetAverageMeasuredValueWindow(v);
    }

    template <bool En = LevelIndicationEnabled, typename = std::enable_if_t<En>>
    CHIP_ERROR SetLevelValue(LevelValueEnum v)
    {
        return mCluster.SetLevelValue(v);
    }

private:
    static constexpr BitFlags<Feature> MakeFeatureFlags()
    {
        BitFlags<Feature> f;
        if constexpr (NumericMeasurementEnabled)
            f.Set(Feature::kNumericMeasurement);
        if constexpr (LevelIndicationEnabled)
            f.Set(Feature::kLevelIndication);
        if constexpr (MediumLevelEnabled)
            f.Set(Feature::kMediumLevel);
        if constexpr (CriticalLevelEnabled)
            f.Set(Feature::kCriticalLevel);
        if constexpr (PeakMeasurementEnabled)
            f.Set(Feature::kPeakMeasurement);
        if constexpr (AverageMeasurementEnabled)
            f.Set(Feature::kAverageMeasurement);
        return f;
    }

    ConcentrationMeasurementCluster mCluster;
    ServerClusterRegistration mRegistration;
};

/**
 * A factory function to create a new instance of a Concentration Measurement Cluster with only the NumericMeasurement feature
 * enabled.
 *
 * @tparam PeakMeasurementEnabled Whether the PeakMeasurement feature is enabled.
 * @tparam AverageMeasurementEnabled Whether the AverageMeasurement feature is enabled.
 * @param endpoint Endpoint that the cluster is on.
 * @param clusterId Cluster that the cluster is on.
 * @param aMeasurementMedium The measurement medium.
 * @param aMeasurementUnit The measurement unit.
 * @return A new instance of Concentration Measurement Cluster.
 */
template <bool PeakMeasurementEnabled, bool AverageMeasurementEnabled>
Instance<true, false, false, false, PeakMeasurementEnabled, AverageMeasurementEnabled>
CreateNumericMeasurementConcentrationCluster(EndpointId endpoint, ClusterId clusterId, MeasurementMediumEnum aMeasurementMedium,
                                             MeasurementUnitEnum aMeasurementUnit)
{
    return Instance<true, false, false, false, PeakMeasurementEnabled, AverageMeasurementEnabled>(
        endpoint, clusterId, aMeasurementMedium, aMeasurementUnit);
}

/**
 * A factory function to create a new instance of a Concentration Measurement Cluster with only the Level Indication feature
 * enabled.
 *
 * @tparam MediumLevelEnabled Whether the MediumLevel feature is enabled.
 * @tparam CriticalLevelEnabled Whether the CriticalLevel feature is enabled.
 * @param endpoint Endpoint that the cluster is on.
 * @param clusterId Cluster that the cluster is on.
 * @param aMeasurementMedium The measurement medium.
 * @return A new instance of Concentration Measurement Cluster.
 */
template <bool MediumLevelEnabled, bool CriticalLevelEnabled>
Instance<false, true, MediumLevelEnabled, CriticalLevelEnabled, false, false>
CreateLevelIndicationConcentrationCluster(EndpointId endpoint, ClusterId clusterId, MeasurementMediumEnum aMeasurementMedium)
{
    return Instance<false, true, MediumLevelEnabled, CriticalLevelEnabled, false, false>(endpoint, clusterId, aMeasurementMedium);
}

/**
 * A factory function to create a new instance of a Concentration Measurement Cluster with both the NumericMeasurement and Level
 * Indication features enabled.
 *
 * @tparam MediumLevelEnabled Whether the MediumLevel feature is enabled.
 * @tparam CriticalLevelEnabled Whether the CriticalLevel feature is enabled.
 * @tparam PeakMeasurementEnabled Whether the PeakMeasurement feature is enabled.
 * @tparam AverageMeasurementEnabled Whether the AverageMeasurement feature is enabled.
 * @param endpoint Endpoint that the cluster is on.
 * @param clusterId Cluster that the cluster is on.
 * @param aMeasurementMedium The measurement medium.
 * @param aMeasurementUnit The measurement unit.
 * @return A new instance of Concentration Measurement Cluster.
 */
template <bool MediumLevelEnabled, bool CriticalLevelEnabled, bool PeakMeasurementEnabled, bool AverageMeasurementEnabled>
Instance<true, true, MediumLevelEnabled, CriticalLevelEnabled, PeakMeasurementEnabled, AverageMeasurementEnabled>
CreateNumericMeasurementAndLevelIndicationConcentrationCluster(EndpointId endpoint, ClusterId clusterId,
                                                               MeasurementMediumEnum aMeasurementMedium,
                                                               MeasurementUnitEnum aMeasurementUnit)
{
    return Instance<true, true, MediumLevelEnabled, CriticalLevelEnabled, PeakMeasurementEnabled, AverageMeasurementEnabled>(
        endpoint, clusterId, aMeasurementMedium, aMeasurementUnit);
}
} // namespace ConcentrationMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
