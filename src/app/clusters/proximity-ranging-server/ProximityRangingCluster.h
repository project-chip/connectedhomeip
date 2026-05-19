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
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/ProximityRanging/AttributeIds.h>
#include <clusters/ProximityRanging/ClusterId.h>
#include <clusters/ProximityRanging/CommandIds.h>
#include <clusters/ProximityRanging/Commands.h>
#include <clusters/ProximityRanging/Enums.h>
#include <clusters/ProximityRanging/Events.h>
#include <clusters/ProximityRanging/Structs.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitMask.h>

#include <optional>

namespace chip {
namespace app {
namespace Clusters {
namespace ProximityRanging {

/**
 * Proximity Ranging Cluster server implementation.
 *
 * Combined DefaultServerCluster implementation. Delegates technology-specific
 * operations to a ProximityRangingDriver provided at construction.
 * All methods must be called from the Matter main thread.
 */
class ProximityRangingCluster : public DefaultServerCluster, public ProximityRangingDriver::Callback
{
public:
    /**
     * Configuration builder for ProximityRangingCluster.
     */
    struct Config
    {
        Config() = default;

        Config & WithFeatures(BitMask<Feature> features)
        {
            mFeatureMap = features;
            return *this;
        }

        BitMask<Feature> mFeatureMap;
    };

    ProximityRangingCluster(EndpointId endpoint, const Config & config) :
        DefaultServerCluster({ endpoint, ProximityRanging::Id }), mFeatureMap(config.mFeatureMap)
    {}

    void SetDriver(ProximityRangingDriver * driver) { mDriver = driver; }
    ProximityRangingDriver * GetDriver() const { return mDriver; }

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

    // Members
    ProximityRangingDriver * mDriver = nullptr;
    const BitMask<Feature> mFeatureMap;

    uint8_t mNextSessionId = 0;
};

} // namespace ProximityRanging
} // namespace Clusters
} // namespace app
} // namespace chip
