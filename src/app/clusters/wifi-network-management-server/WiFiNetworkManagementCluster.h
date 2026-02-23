/**
 *
 *    Copyright (c) 2024-2026 Project CHIP Authors
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
#include <clusters/WiFiNetworkManagement/ClusterId.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/FixedBuffer.h>
#include <lib/support/Span.h>

namespace chip::app::Clusters {

class WiFiNetworkManagementCluster : public DefaultServerCluster
{
public:
    WiFiNetworkManagementCluster(EndpointId endpoint) : DefaultServerCluster({ endpoint, WiFiNetworkManagement::Id }) {}

    WiFiNetworkManagementCluster(WiFiNetworkManagementCluster const &)             = delete;
    WiFiNetworkManagementCluster & operator=(WiFiNetworkManagementCluster const &) = delete;

    /// Sets the Wi-Fi credentials to expose via this cluster.
    /// The application is responsible for calling this method during startup,
    /// and whenever the Wi-Fi credentials are modified externally.
    CHIP_ERROR SetNetworkCredentials(ByteSpan ssid, ByteSpan passphrase);
    CHIP_ERROR ClearNetworkCredentials();

    /// Returns true if the cluster holds a valid SSID and Passphrase.
    bool HasNetworkCredentials() const { return !mSsid.empty(); }

    /// Returns the SSID of the Wi-Fi network managed by this cluster,
    // or an empty span if no current network credentials are set.
    ByteSpan Ssid() const { return mSsid.span(); }

    /// Returns the passphrase for the Wi-Fi network managed by this cluster,
    // or an empty span if there are no current network credentials.
    ByteSpan Passphrase() const { return mPassphrase.Span(); }

    // ServerClusterInterface overrides

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;

private:
    FixedByteBuffer<32, uint8_t> mSsid;
    uint64_t mPassphraseSurrogate = 0;
    Crypto::SensitiveDataBuffer<64> mPassphrase;
};

} // namespace chip::app::Clusters
