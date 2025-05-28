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
#include "NetworkCommissioningCluster.h"

#include <app/server-cluster/DefaultServerCluster.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <protocols/interaction_model/StatusCode.h>

#include <optional>

namespace chip {
namespace app {
namespace Clusters {

/// Integration of Network Commissioning logic within the Matter data model
///
/// Translates between matter calls and Network commissioning logic
class NetworkCommissioningCluster : public DefaultServerCluster
{
public:
    NetworkCommissioningCluster(EndpointId endpointId) : DefaultServerCluster({ endpointId, SoftwareDiagnostics::Id }) {}

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override
    {
        // FIXME: implement
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override
    {
        // FIXME: implement
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override
    {
        // FIXME: implement
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override
    {
        // FIXME: implement
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
};

} // namespace Clusters
} // namespace app
} // namespace chip
