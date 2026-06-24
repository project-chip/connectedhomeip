/*
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

#include <app/clusters/proximity-ranging-server/ProximityRangingDriver.h>
#include <app/clusters/proximity-ranging-server/RangingAdapter.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/ProximityRanging/ClusterId.h>
#include <clusters/ProximityRanging/Commands.h>
#include <clusters/ProximityRanging/Enums.h>
#include <clusters/ProximityRanging/Structs.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitMask.h>
#include <lib/support/Span.h>
#include <lib/support/TimerDelegate.h>

#include <optional>

namespace chip {
namespace app {
namespace Clusters {
namespace ProximityRanging {

/**
 * Proximity Ranging Cluster server implementation.
 *
 * Owns its ProximityRangingDriver as a private member, constructed from the
 * adapter set and TimerDelegate supplied via Config. This makes each cluster
 * instance independent — there is no shared driver across endpoints, so
 * multi-endpoint Proximity Ranging deployments are well-defined: each
 * endpoint has its own session pool, its own callback wiring, and its own
 * (exclusive) set of adapters.
 *
 * Adapter exclusivity: each RangingAdapter is registered as a callback
 * recipient with exactly ONE driver instance. The application is therefore
 * responsible for assigning each adapter to at most one cluster instance.
 *
 * All methods must be called from the Matter main thread.
 */
class ProximityRangingCluster : public DefaultServerCluster, public ProximityRangingDriver::Callback
{
public:
    /**
     * Configuration builder for ProximityRangingCluster.
     *
     * Mandatory inputs:
     *   - TimerDelegate &: drives the per-session StartTime / EndTime /
     *     RangingInstanceInterval scheduling owned by the cluster's
     *     internal driver. Must outlive the cluster.
     *
     * Optional builders:
     *   - WithFeatures(): sets the cluster's feature map. Defaults to empty.
     *   - WithAdapters(): supplies the technology adapter set the cluster's
     *     driver routes ranging operations to. The backing array must outlive
     *     the cluster. Defaults to an empty span (cluster reads attributes
     *     and reports capabilities, but cannot start any ranging session).
     */
    struct Config
    {
        Config(TimerDelegate & timerDelegate) : mTimerDelegate(timerDelegate) {}

        Config & WithFeatures(BitMask<Feature> features)
        {
            mFeatureMap = features;
            return *this;
        }

        Config & WithAdapters(Span<RangingAdapter * const> adapters)
        {
            mAdapters = adapters;
            return *this;
        }

        TimerDelegate & mTimerDelegate;
        BitMask<Feature> mFeatureMap;
        Span<RangingAdapter * const> mAdapters;
    };

    ProximityRangingCluster(EndpointId endpoint, const Config & config) :
        DefaultServerCluster({ endpoint, ProximityRanging::Id }), mDriver(config.mAdapters, config.mTimerDelegate),
        mFeatureMap(config.mFeatureMap)
    {}

    // DefaultServerCluster implementation
    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown(ClusterShutdownType shutdownType) override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;

    // ProximityRangingDriver::Callback implementation
    void OnMeasurementData(uint8_t sessionId, const Structs::RangingMeasurementDataStruct::Type & measurement) override;
    void OnSessionStopped(uint8_t sessionId, RangingSessionStatusEnum status) override;
    void OnAttributeChanged(AttributeId attributeId) override;

private:
    // Command handlers
    std::optional<DataModel::ActionReturnStatus> HandleStartRangingRequest(const DataModel::InvokeRequest & request,
                                                                           TLV::TLVReader & reader, CommandHandler * handler);
    DataModel::ActionReturnStatus HandleStopRangingRequest(const DataModel::InvokeRequest & request, TLV::TLVReader & reader);

    // Session ID generation
    uint8_t GenerateSessionId();

    // Spec preflight: reject before touching the driver if the requested technology
    // is not in FeatureMap or its matching DeviceRoleConfig field is missing/inconsistent.
    ResultCodeEnum ValidateStartRangingRequest(const Commands::StartRangingRequest::DecodableType & request) const;

    // Members
    ProximityRangingDriver mDriver;
    bool mDriverInitialized = false;
    const BitMask<Feature> mFeatureMap;

    uint8_t mNextSessionId = 0;
};

} // namespace ProximityRanging
} // namespace Clusters
} // namespace app
} // namespace chip
