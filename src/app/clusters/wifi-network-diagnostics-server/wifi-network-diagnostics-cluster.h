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

#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/WiFiNetworkDiagnostics/ClusterId.h>
#include <clusters/WiFiNetworkDiagnostics/Enums.h>
#include <clusters/WiFiNetworkDiagnostics/Metadata.h>
#include <lib/core/DataModelTypes.h>
#include <platform/DiagnosticDataProvider.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {

struct WiFiNetworkDiagnosticsEnabledAttributes
{
    bool enableBeaconLostCount : 1;
    bool enableBeaconRxCount : 1;
    bool enablePacketMulticastRxCount : 1;
    bool enablePacketMulticastTxCount : 1;
    bool enablePacketUnicastRxCount : 1;
    bool enablePacketUnicastTxCount : 1;
    bool enableCurrentMaxRate : 1;
    bool enableOverrunCount : 1;
};

class WiFiDiagnosticsServerLogic : public DeviceLayer::WiFiDiagnosticsDelegate
{
public:
    WiFiDiagnosticsServerLogic(DeviceLayer::DiagnosticDataProvider & diagnosticProvider,
                               const WiFiNetworkDiagnosticsEnabledAttributes & enabledAttributes,
                               BitFlags<WiFiNetworkDiagnostics::Feature> featureFlags) :
        mDiagnosticProvider(diagnosticProvider), mEnabledAttributes(enabledAttributes), mFeatureFlags(featureFlags)
    {}

    template <typename T, typename Type>
    CHIP_ERROR ReadIfSupported(CHIP_ERROR (DeviceLayer::DiagnosticDataProvider::*getter)(T &), Type & data,
                               AttributeValueEncoder & aEncoder);

    CHIP_ERROR ReadWiFiBssId(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadSecurityType(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadWiFiVersion(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadChannelNumber(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadWiFiRssi(AttributeValueEncoder & aEncoder);

#ifdef WI_FI_NETWORK_DIAGNOSTICS_ENABLE_RESET_COUNTS_CMD
    CHIP_ERROR HandleResetCounts()
    {
        mDiagnosticProvider.ResetWiFiNetworkDiagnosticsCounts();
        return CHIP_NO_ERROR;
    }
#endif

    // Gets called when the Node detects Node's Wi-Fi connection has been disconnected.
    void OnDisconnectionDetected(uint16_t reasonCode) override;

    // Gets called when the Node fails to associate or authenticate an access point.
    void OnAssociationFailureDetected(uint8_t associationFailureCause, uint16_t status) override;

    // Gets when the Node's connection status to a Wi-Fi network has changed.
    void OnConnectionStatusChanged(uint8_t connectionStatus) override;

    // Getter methods for private members
    const BitFlags<WiFiNetworkDiagnostics::Feature> & GetFeatureFlags() const { return mFeatureFlags; }
    const WiFiNetworkDiagnosticsEnabledAttributes & GetEnabledAttributes() const { return mEnabledAttributes; }

private:
    DeviceLayer::DiagnosticDataProvider & mDiagnosticProvider;
    const WiFiNetworkDiagnosticsEnabledAttributes mEnabledAttributes;
    const BitFlags<WiFiNetworkDiagnostics::Feature> mFeatureFlags;
};

class WiFiDiagnosticsServer : public DefaultServerCluster
{
public:
    WiFiDiagnosticsServer(EndpointId endpointId, DeviceLayer::DiagnosticDataProvider & diagnosticProvider,
                          const WiFiNetworkDiagnosticsEnabledAttributes & enabledAttributes,
                          BitFlags<WiFiNetworkDiagnostics::Feature> featureFlags) :
        DefaultServerCluster({ endpointId, WiFiNetworkDiagnostics::Id }),
        mLogic(diagnosticProvider, enabledAttributes, featureFlags)
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
