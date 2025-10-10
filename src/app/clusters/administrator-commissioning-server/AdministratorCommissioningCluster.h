/*
 *    Copyright (c) 2021-2025 Project CHIP Authors
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

#include <app/clusters/administrator-commissioning-server/AdministratorCommissioningLogic.h>

#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/AdministratorCommissioning/ClusterId.h>

namespace chip {
namespace app {
namespace Clusters {

class AdministratorCommissioningCluster : public DefaultServerCluster
{
public:
    AdministratorCommissioningCluster(EndpointId endpointId, BitFlags<AdministratorCommissioning::Feature> _unused_features) :
        DefaultServerCluster({ endpointId, AdministratorCommissioning::Id })
    {}

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

protected:
    AdministratorCommissioningLogic mLogic;
};

/// Adds support for `OpenBasicCommissioningWindow` command to the AdministratorCommissioningWindow
class AdministratorCommissioningWithBasicCommissioningWindowCluster : public AdministratorCommissioningCluster
{
public:
    AdministratorCommissioningWithBasicCommissioningWindowCluster(EndpointId endpointId,
                                                                  BitFlags<AdministratorCommissioning::Feature> features) :
        AdministratorCommissioningCluster(endpointId, features),
        mFeatures(features)
    {}

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;

private:
    const BitFlags<AdministratorCommissioning::Feature> mFeatures;
};

} // namespace Clusters
} // namespace app
} // namespace chip
