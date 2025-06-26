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

#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/EthernetNetworkDiagnostics/ClusterId.h>
#include <clusters/EthernetNetworkDiagnostics/Commands.h>
#include <clusters/EthernetNetworkDiagnostics/Enums.h>
#include <clusters/EthernetNetworkDiagnostics/Events.h>
#include <clusters/EthernetNetworkDiagnostics/Metadata.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/Span.h>
#include <platform/DiagnosticDataProvider.h>
#include <protocols/interaction_model/StatusCode.h>
#include <sys/types.h>

namespace chip {
namespace app {
namespace Clusters {

struct EthernetDiagnosticsEnabledAttributes
{
    bool enableCarrierDetect : 1;
    bool enableFullDuplex : 1;
    bool enablePacketCount : 1;
    bool enablePHYRate : 1;
    bool enableTimeSinceReset : 1;
    bool enableErrCount : 1;
};

/// Integration of Ethernet network diagnostics within the Matter data model
/// This cluster is expected to only ever exist on endpoint 0 as it is a singleton cluster.
class EthernetDiagnosticsServerCluster : public DefaultServerCluster
{
public:
    EthernetDiagnosticsServerCluster(DeviceLayer::DiagnosticDataProvider & provider,
                                     const EthernetDiagnosticsEnabledAttributes & enabledAttributes);

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

private:
    CHIP_ERROR ReadPHYRate(AttributeValueEncoder & encoder) const;
    CHIP_ERROR ReadFullDuplex(AttributeValueEncoder & encoder) const;
    CHIP_ERROR ReadCarrierDetect(AttributeValueEncoder & encoder) const;
    BitFlags<EthernetNetworkDiagnostics::Feature> GetFeatureMap() const;
    CHIP_ERROR EncodeValue(uint64_t value, CHIP_ERROR readError, AttributeValueEncoder & encoder);

    DeviceLayer::DiagnosticDataProvider & mProvider;
    const EthernetDiagnosticsEnabledAttributes mEnabledAttributes;
};

} // namespace Clusters
} // namespace app
} // namespace chip
