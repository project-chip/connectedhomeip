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
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ConcentrationMeasurement {

/**
 * Compatibility shim over ConcentrationMeasurementCluster that preserves the
 * original template-based API.
 *
 * Migration notes:
 *  - Init() registers with CodegenDataModelProvider for backward compatibility.
 *  - Prefer explicit registry.Register(instance.Registration()) in new code.
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
        mEndpointId(aEndpointId),
        mConfig{ aClusterId, MakeFeatureFlags(), aMeasurementMedium, MeasurementUnitEnum::kUnknownEnumValue }
    {}

    // Constructor for clusters with kNumericMeasurement.
    Instance(EndpointId aEndpointId, ClusterId aClusterId, MeasurementMediumEnum aMeasurementMedium,
             MeasurementUnitEnum aMeasurementUnit) :
        mEndpointId(aEndpointId),
        mConfig{ aClusterId, MakeFeatureFlags(), aMeasurementMedium, aMeasurementUnit }
    {}

    ~Instance()
    {
        if (mCluster.IsConstructed())
        {
            LogErrorOnFailure(CodegenDataModelProvider::Instance().Registry().Unregister(&mCluster.Cluster()));
            mCluster.Destroy();
        }
    }

    /**
     * Registers this cluster with the CodegenDataModelProvider registry.
     * Kept for backward compatibility with existing emberAfXxx Init callback sites.
     */
    CHIP_ERROR Init()
    {
        VerifyOrReturnValue(!mCluster.IsConstructed(), CHIP_NO_ERROR);
        mCluster.Create(mEndpointId, mConfig);
        CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration());
        if (err != CHIP_NO_ERROR)
        {
            mCluster.Destroy();
            return err;
        }
        return CHIP_NO_ERROR;
    }

    ServerClusterRegistration & Registration() { return mCluster.Registration(); }

    // Fixed-value setters: must be called before Init(). These configure the cluster's
    // immutable attributes (min/max range, uncertainty) via the Config passed to the constructor.
    template <bool En = NumericMeasurementEnabled, typename = std::enable_if_t<En>>
    CHIP_ERROR SetMinMeasuredValue(DataModel::Nullable<float> v)
    {
        VerifyOrReturnError(!mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);
        mConfig.minMeasured = v;
        return CHIP_NO_ERROR;
    }

    template <bool En = NumericMeasurementEnabled, typename = std::enable_if_t<En>>
    CHIP_ERROR SetMaxMeasuredValue(DataModel::Nullable<float> v)
    {
        VerifyOrReturnError(!mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);
        mConfig.maxMeasured = v;
        return CHIP_NO_ERROR;
    }

    template <bool En = NumericMeasurementEnabled, typename = std::enable_if_t<En>>
    CHIP_ERROR SetUncertainty(float v)
    {
        VerifyOrReturnError(!mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);
        mConfig.uncertainty = v;
        return CHIP_NO_ERROR;
    }

    // Set*() methods are needed to push sensor readings by the device-app hooks.
    // remote Matter clients can't use these setters and write attributes.
    // The access boundary is enforced by not implementing WriteAttribute()
    template <bool En = NumericMeasurementEnabled, typename = std::enable_if_t<En>>
    CHIP_ERROR SetMeasuredValue(DataModel::Nullable<float> v)
    {
        VerifyOrReturnError(mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);

        return mCluster.Cluster().SetMeasuredValue(v);
    }

    template <bool En = PeakMeasurementEnabled, typename = std::enable_if_t<En>>
    CHIP_ERROR SetPeakMeasuredValue(DataModel::Nullable<float> v)
    {
        VerifyOrReturnError(mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);

        return mCluster.Cluster().SetPeakMeasuredValue(v);
    }

    template <bool En = PeakMeasurementEnabled, typename = std::enable_if_t<En>>
    CHIP_ERROR SetPeakMeasuredValueWindow(uint32_t v)
    {
        VerifyOrReturnError(mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);

        return mCluster.Cluster().SetPeakMeasuredValueWindow(v);
    }

    template <bool En = AverageMeasurementEnabled, typename = std::enable_if_t<En>>
    CHIP_ERROR SetAverageMeasuredValue(DataModel::Nullable<float> v)
    {
        VerifyOrReturnError(mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);

        return mCluster.Cluster().SetAverageMeasuredValue(v);
    }

    template <bool En = AverageMeasurementEnabled, typename = std::enable_if_t<En>>
    CHIP_ERROR SetAverageMeasuredValueWindow(uint32_t v)
    {
        VerifyOrReturnError(mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);

        return mCluster.Cluster().SetAverageMeasuredValueWindow(v);
    }

    template <bool En = LevelIndicationEnabled, typename = std::enable_if_t<En>>
    CHIP_ERROR SetLevelValue(LevelValueEnum v)
    {
        VerifyOrReturnError(mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);

        return mCluster.Cluster().SetLevelValue(v);
    }

private:
    static constexpr BitFlags<Feature> MakeFeatureFlags()
    {
        BitFlags<Feature> f;
        f.Set(Feature::kNumericMeasurement, NumericMeasurementEnabled);
        f.Set(Feature::kLevelIndication, LevelIndicationEnabled);
        f.Set(Feature::kMediumLevel, MediumLevelEnabled);
        f.Set(Feature::kCriticalLevel, CriticalLevelEnabled);
        f.Set(Feature::kPeakMeasurement, PeakMeasurementEnabled);
        f.Set(Feature::kAverageMeasurement, AverageMeasurementEnabled);
        return f;
    }

    EndpointId mEndpointId;
    ConcentrationMeasurementCluster::Config mConfig;
    LazyRegisteredServerCluster<ConcentrationMeasurementCluster> mCluster;
};

template <bool PeakMeasurementEnabled, bool AverageMeasurementEnabled>
Instance<true, false, false, false, PeakMeasurementEnabled, AverageMeasurementEnabled>
CreateNumericMeasurementConcentrationCluster(EndpointId endpoint, ClusterId clusterId, MeasurementMediumEnum aMeasurementMedium,
                                             MeasurementUnitEnum aMeasurementUnit)
{
    return Instance<true, false, false, false, PeakMeasurementEnabled, AverageMeasurementEnabled>(
        endpoint, clusterId, aMeasurementMedium, aMeasurementUnit);
}

template <bool MediumLevelEnabled, bool CriticalLevelEnabled>
Instance<false, true, MediumLevelEnabled, CriticalLevelEnabled, false, false>
CreateLevelIndicationConcentrationCluster(EndpointId endpoint, ClusterId clusterId, MeasurementMediumEnum aMeasurementMedium)
{
    return Instance<false, true, MediumLevelEnabled, CriticalLevelEnabled, false, false>(endpoint, clusterId, aMeasurementMedium);
}

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
