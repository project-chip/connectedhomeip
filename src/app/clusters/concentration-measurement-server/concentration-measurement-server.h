/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "mode-base-cluster-objects.h"
#include <app/AttributeAccessInterface.h>
#include <app/util/af.h>
#include <lib/support/IntrusiveList.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ConcentrationMeasurement {

class Instance : public AttributeAccessInterface
{
private:
    EndpointId mEndpointId{};
    ClusterId mClusterId{};

    // Attribute data store
    uint32_t mFeature = 0;
    DataModel::Nullable<float> mMeasuredValue;
    DataModel::Nullable<float> mMinMeasuredValue;
    DataModel::Nullable<float> mMaxMeasuredValue;
    DataModel::Nullable<float> mPeakMeasuredValue;
    uint32_t mPeakMeasuredValueWindow = 0;
    DataModel::Nullable<float> mAverageMeasuredValue;
    uint32_t mAverageMeasuredValueWindow = 0;
    float mUncertainty                   = 0.0;
    MeasurementUnitEnum mMeasurementUnit;
    MeasurementMediumEnum mMeasurementMedium;
    LevelValueEnum mLevel;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

public:
    /**
     * Creates a mode base cluster instance. The Init() function needs to be called for this instance to be registered and
     * called by the interaction model at the appropriate times.
     * @param aEndpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aClusterId The ID of the ModeBase aliased cluster to be instantiated.
     * @param aFeature The bitmask value that identifies which features are supported by this instance.
     */
    Instance(EndpointId aEndpointId, ClusterId aClusterId, uint32_t aFeature);

    /**
     * Returns true if the feature is supported.
     * @param feature the feature to check.
     */
    bool HasFeature(Feature feature) const;

    /**
     * @return The endpoint ID.
     */
    EndpointId GetEndpointId() const { return mEndpointId; }
};

} // namespace ConcentrationMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip