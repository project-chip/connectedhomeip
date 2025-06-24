/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/ethernet-network-diagnostics-server/ethernet-diagnostics-logic.h>

#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/EthernetNetworkDiagnostics/ClusterId.h>
#include <clusters/EthernetNetworkDiagnostics/Commands.h>
#include <clusters/EthernetNetworkDiagnostics/Events.h>
#include <clusters/EthernetNetworkDiagnostics/Metadata.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/Span.h>
#include <protocols/interaction_model/StatusCode.h>
#include <sys/types.h>

namespace chip {
namespace app {
namespace Clusters {

/// Integration of Ethernet network diagnostics logic within the Matter data model
///
/// Translates between matter calls and Ethernet Network Diagnostics logic
///
/// This cluster is expected to only ever exist on endpoint 0 as it is a singleton cluster.
template <typename LOGIC>
class EthernetDiagnosticsServerCluster : public DefaultServerCluster, private LOGIC
{
public:
    template <typename... Args>
    EthernetDiagnosticsServerCluster(Args &&... args) :
        DefaultServerCluster({ kRootEndpointId, EthernetNetworkDiagnostics::Id }), LOGIC(std::forward<Args>(args)...)
    {}

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override
    {
        switch (request.path.mAttributeId)
        {
        case EthernetNetworkDiagnostics::Attributes::PHYRate::Id:
            return LOGIC::ReadPHYRate(encoder);
        case EthernetNetworkDiagnostics::Attributes::FullDuplex::Id:
            return LOGIC::ReadFullDuplex(encoder);
        case EthernetNetworkDiagnostics::Attributes::CarrierDetect::Id:
            return LOGIC::ReadCarrierDetect(encoder);
        case EthernetNetworkDiagnostics::Attributes::TimeSinceReset::Id: {
            uint64_t value;
            CHIP_ERROR err = LOGIC::GetTimeSinceReset(value);
            return EncodeValue(value, err, encoder);
        }
        case EthernetNetworkDiagnostics::Attributes::PacketRxCount::Id: {
            uint64_t value;
            CHIP_ERROR err = LOGIC::GetPacketRxCount(value);
            return EncodeValue(value, err, encoder);
        }
        case EthernetNetworkDiagnostics::Attributes::PacketTxCount::Id: {
            uint64_t value;
            CHIP_ERROR err = LOGIC::GetPacketTxCount(value);
            return EncodeValue(value, err, encoder);
        }
        case EthernetNetworkDiagnostics::Attributes::TxErrCount::Id: {
            uint64_t value;
            CHIP_ERROR err = LOGIC::GetTxErrCount(value);
            return EncodeValue(value, err, encoder);
        }
        case EthernetNetworkDiagnostics::Attributes::CollisionCount::Id: {
            uint64_t value;
            CHIP_ERROR err = LOGIC::GetCollisionCount(value);
            return EncodeValue(value, err, encoder);
        }
        case EthernetNetworkDiagnostics::Attributes::OverrunCount::Id: {
            uint64_t value;
            CHIP_ERROR err = LOGIC::GetOverrunCount(value);
            return EncodeValue(value, err, encoder);
        }
        case Globals::Attributes::FeatureMap::Id:
            return encoder.Encode(LOGIC::GetFeatureMap().Raw());
        case Globals::Attributes::ClusterRevision::Id:
            return encoder.Encode(EthernetNetworkDiagnostics::kRevision);
        default:
            return Protocols::InteractionModel::Status::UnsupportedAttribute;
        }
    }

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override
    {
        switch (request.path.mCommandId)
        {
        case EthernetNetworkDiagnostics::Commands::ResetCounts::Id:
            return LOGIC::ResetCounts();
        default:
            return Protocols::InteractionModel::Status::UnsupportedCommand;
        }
    }

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override
    {
        return LOGIC::AcceptedCommands(builder);
    }

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override
    {
        return LOGIC::Attributes(builder);
    }

private:
    // Encodes the `value` in `encoder`, while handling a potential `readError` that occured
    // when the input `value` was read:
    //   - CHIP_ERROR_NOT_IMPLEMENTED results in a 0 being encoded
    //   - any other read error is just forwarded
    CHIP_ERROR EncodeValue(uint64_t value, CHIP_ERROR readError, AttributeValueEncoder & encoder)
    {
        if (readError == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
        {
            value = 0;
        }
        else if (readError != CHIP_NO_ERROR)
        {
            return readError;
        }
        return encoder.Encode(value);
    }
};

} // namespace Clusters
} // namespace app
} // namespace chip
