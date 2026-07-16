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

#include <app/AttributeValueEncoder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/WiFiNetworkDiagnostics/Attributes.h>
#include <clusters/WiFiNetworkDiagnostics/ClusterId.h>
#include <clusters/WiFiNetworkDiagnostics/Enums.h>
#include <lib/core/DataModelTypes.h>
#include <platform/DiagnosticDataProvider.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {

class WiFiDiagnosticsServerCluster : public DefaultServerCluster, public DeviceLayer::WiFiDiagnosticsDelegate
{
public:
    // NOTE: this set is smaller than the full optional attributes supported by diagnostics
    //       as other attributes are controlled by feature flags
    using OptionalAttributeSet = chip::app::OptionalAttributeSet<WiFiNetworkDiagnostics::Attributes::CurrentMaxRate::Id>;

    WiFiDiagnosticsServerCluster(EndpointId endpointId, DeviceLayer::DiagnosticDataProvider & diagnosticProvider,
                                 const OptionalAttributeSet & optionalAttributeSet,
                                 BitFlags<WiFiNetworkDiagnostics::Feature> featureFlags) :
        DefaultServerCluster({ endpointId, WiFiNetworkDiagnostics::Id }),
        mEndpointId(endpointId), mDiagnosticProvider(diagnosticProvider), mOptionalAttributeSet(optionalAttributeSet),
        mFeatureFlags(featureFlags)
    {
        mDiagnosticProvider.SetWiFiDiagnosticsDelegate(this);
    }

    ~WiFiDiagnosticsServerCluster() { mDiagnosticProvider.SetWiFiDiagnosticsDelegate(nullptr); }

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

    // DeviceLayer::WiFiDiagnosticsDelegate implementation
    void OnDisconnectionDetected(uint16_t reasonCode) override;
    void OnAssociationFailureDetected(uint8_t associationFailureCause, uint16_t status) override;
    void OnConnectionStatusChanged(uint8_t connectionStatus) override;

private:
    EndpointId mEndpointId;
    DeviceLayer::DiagnosticDataProvider & mDiagnosticProvider;
    const OptionalAttributeSet mOptionalAttributeSet;
    const BitFlags<WiFiNetworkDiagnostics::Feature> mFeatureFlags;
};

} // namespace Clusters
} // namespace app
} // namespace chip
