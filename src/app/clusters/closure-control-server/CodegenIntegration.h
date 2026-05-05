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

#include <app/clusters/closure-control-server/ClosureControlCluster.h>
#include <app/clusters/closure-control-server/ClosureControlClusterDelegate.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureControl {

/**
 * @brief Structure is used to configure and validate the Cluster configuration.
 *        Validates if the feature map, attributes and commands configuration is valid.
 */
struct ClusterConformance
{
public:
    BitFlags<Feature> & FeatureMap() { return mFeatureMap; }
    const BitFlags<Feature> & FeatureMap() const { return mFeatureMap; }

    OptionalAttributesSet & OptionalAttributes() { return mOptionalAttributes; }
    const OptionalAttributesSet & OptionalAttributes() const { return mOptionalAttributes; }

    inline bool HasFeature(Feature aFeature) const { return mFeatureMap.Has(aFeature); }

private:
    BitFlags<Feature> mFeatureMap;
    OptionalAttributesSet mOptionalAttributes;
};

/**
 * @brief Struct to store the cluster initialization parameters
 */
struct ClusterInitParameters
{
    MainStateEnum mMainState                                             = MainStateEnum::kStopped;
    DataModel::Nullable<GenericOverallCurrentState> mOverallCurrentState = DataModel::NullNullable;
    BitFlags<LatchControlModesBitmap> mLatchControlModes;
};

/**
 * @brief Interface owns the lifecycle of a ClosureControlCluster instance and registers it with
 *        the data model provider.
 */
class Interface
{
public:
    Interface(EndpointId endpoint, ClosureControlClusterDelegate & delegate);
    ~Interface() = default;

    Interface(const Interface &)             = delete;
    Interface & operator=(const Interface &) = delete;

    /**
     * @brief Constructs the underlying cluster from the given conformance and initialization
     *        parameters and registers it with the data model provider.
     *
     * @return CHIP_NO_ERROR on success.
     *         CHIP_ERROR_INCORRECT_STATE if the cluster is already constructed.
     */
    CHIP_ERROR Init(const ClusterConformance & conformance, const ClusterInitParameters & initParams);

    /**
     * @brief Unregisters the cluster from the data model provider and destroys the instance.
     */
    CHIP_ERROR Shutdown();

    /**
     * @brief Returns a reference to the underlying cluster instance.
     *        Must only be called after Init() has been invoked successfully.
     */
    ClosureControlCluster & Cluster();

private:
    EndpointId mEndpoint;
    ClosureControlClusterDelegate & mDelegate;
    chip::app::LazyRegisteredServerCluster<ClosureControlCluster> mCluster;
};

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip
