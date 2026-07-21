/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/clusters/water-heater-management-server/Delegate.h>
#include <app/server-cluster/DefaultServerCluster.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <lib/core/CHIPError.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WaterHeaterManagement {

class WaterHeaterManagementCluster : public DefaultServerCluster
{
public:
    WaterHeaterManagementCluster(EndpointId aEndpointId, Delegate & aDelegate, Feature aFeature);

    ~WaterHeaterManagementCluster();

    bool HasFeature(Feature aFeature) const;

    CHIP_ERROR GenerateBoostStartedEvent(uint32_t durationSecs, Optional<bool> oneShot, Optional<bool> emergencyBoost,
                                         Optional<int16_t> temporarySetpoint, Optional<Percent> targetPercentage,
                                         Optional<Percent> targetReheat);
    CHIP_ERROR GenerateBoostEndedEvent();

private:
    Delegate & mDelegate;
    BitMask<Feature> mFeature;

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    Protocols::InteractionModel::Status HandleBoost(const Commands::Boost::DecodableType & commandData);
    Protocols::InteractionModel::Status HandleCancelBoost(const Commands::CancelBoost::DecodableType & commandData);
};

} // namespace WaterHeaterManagement
} // namespace Clusters
} // namespace app
} // namespace chip
