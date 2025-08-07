/*
 *
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

#include "app/server-cluster/OptionalAttributes.h"
#include <app/AttributeValueEncoder.h>
#include <clusters/WiFiNetworkDiagnostics/Enums.h>
#include <lib/core/DataModelTypes.h>
#include <platform/DiagnosticDataProvider.h>

namespace chip::app {

ATTRIBUTE_BITS_MARK_OPTIONAL(WiFiNetworkDiagnostics, CurrentMaxRate);

} // namespace chip::app

namespace chip {
namespace app {
namespace Clusters {

class WiFiDiagnosticsServerLogic : public DeviceLayer::WiFiDiagnosticsDelegate
{
public:
    WiFiDiagnosticsServerLogic(EndpointId endpointId, DeviceLayer::DiagnosticDataProvider & diagnosticProvider,
                               const ClusterAttributeBits<WiFiNetworkDiagnostics::Id> & enabledAttributes,
                               BitFlags<WiFiNetworkDiagnostics::Feature> featureFlags) :
        mEndpointId(endpointId), mDiagnosticProvider(diagnosticProvider), mEnabledAttributes(enabledAttributes),
        mFeatureFlags(featureFlags)
    {
        mDiagnosticProvider.SetWiFiDiagnosticsDelegate(this);
    }

    ~WiFiDiagnosticsServerLogic() { mDiagnosticProvider.SetWiFiDiagnosticsDelegate(nullptr); }

    template <typename T, typename Type>
    CHIP_ERROR ReadIfSupported(CHIP_ERROR (DeviceLayer::DiagnosticDataProvider::*getter)(T &), Type & data,
                               AttributeValueEncoder & aEncoder)
    {
        T value;
        CHIP_ERROR err = (mDiagnosticProvider.*getter)(value);

        if (err == CHIP_NO_ERROR)
        {
            data.SetNonNull(value);
        }
        else
        {
            ChipLogProgress(Zcl, "The WiFi interface is not currently configured or operational.");
        }

        return aEncoder.Encode(data);
    }

    // WiFiBssId uses custom implementations instead of ReadIfSupported because it
    // is attribute of type octet string.
    CHIP_ERROR ReadWiFiBssId(AttributeValueEncoder & aEncoder);

    CHIP_ERROR HandleResetCounts()
    {
        mDiagnosticProvider.ResetWiFiNetworkDiagnosticsCounts();
        return CHIP_NO_ERROR;
    }

    // Gets called when the Node detects Node's Wi-Fi connection has been disconnected.
    void OnDisconnectionDetected(uint16_t reasonCode) override;

    // Gets called when the Node fails to associate or authenticate an access point.
    void OnAssociationFailureDetected(uint8_t associationFailureCause, uint16_t status) override;

    // Gets when the Node's connection status to a Wi-Fi network has changed.
    void OnConnectionStatusChanged(uint8_t connectionStatus) override;

    // Getter methods for private members
    EndpointId GetEndpointId() const { return mEndpointId; }
    const BitFlags<WiFiNetworkDiagnostics::Feature> & GetFeatureFlags() const { return mFeatureFlags; }
    const AttributeBits & GetEnabledAttributes() const { return mEnabledAttributes; }

private:
    EndpointId mEndpointId;
    DeviceLayer::DiagnosticDataProvider & mDiagnosticProvider;
    const AttributeBits mEnabledAttributes;
    const BitFlags<WiFiNetworkDiagnostics::Feature> mFeatureFlags;
};

} // namespace Clusters
} // namespace app
} // namespace chip
