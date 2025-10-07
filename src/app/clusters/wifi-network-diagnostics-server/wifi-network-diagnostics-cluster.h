/*
 *
 *    Copyright (c) 2024-25 Project CHIP Authors
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

#include <app/clusters/wifi-network-diagnostics-server/wifi-network-diagnostics-logic.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/WiFiNetworkDiagnostics/ClusterId.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {

class WiFiDiagnosticsServerCluster : public DefaultServerCluster
{
public:
    WiFiDiagnosticsServerCluster(EndpointId endpointId, DeviceLayer::DiagnosticDataProvider & diagnosticProvider,
                                 const WiFiDiagnosticsServerLogic::OptionalAttributeSet & optionalAttributeSet,
                                 BitFlags<WiFiNetworkDiagnostics::Feature> featureFlags) :
        DefaultServerCluster({ endpointId, WiFiNetworkDiagnostics::Id }),
        mLogic(endpointId, diagnosticProvider, optionalAttributeSet, featureFlags)
    {}

    // Server cluster implementation

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

    WiFiDiagnosticsServerLogic & GetLogic() { return mLogic; }

private:
    WiFiDiagnosticsServerLogic mLogic;
};

} // namespace Clusters
} // namespace app
} // namespace chip
