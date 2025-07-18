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

#include <clusters/WiFiNetworkDiagnostics/Enums.h>
#include <platform/DiagnosticDataProvider.h>
#include <app/AttributeValueEncoder.h>

namespace chip {
namespace app {
namespace Clusters {

struct WiFiNetworkDiagnosticsEnabledAttributes
{
    bool enableCurrentMaxRate : 1;
};

class WiFiDiagnosticsServerLogic : public DeviceLayer::WiFiDiagnosticsDelegate
{
public:
    WiFiDiagnosticsServerLogic(DeviceLayer::DiagnosticDataProvider & diagnosticProvider,
                               const WiFiNetworkDiagnosticsEnabledAttributes & enabledAttributes,
                               BitFlags<WiFiNetworkDiagnostics::Feature> featureFlags) :
        mDiagnosticProvider(diagnosticProvider),
        mEnabledAttributes(enabledAttributes), mFeatureFlags(featureFlags)
    {}

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

    // These attributes use custom implementations instead of ReadIfSupported because they
    // provide more detailed logging messages and, in some cases, additional metrics.
    CHIP_ERROR ReadWiFiBssId(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadSecurityType(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadWiFiVersion(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadChannelNumber(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadWiFiRssi(AttributeValueEncoder & aEncoder);

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
    const BitFlags<WiFiNetworkDiagnostics::Feature> & GetFeatureFlags() const { return mFeatureFlags; }
    const WiFiNetworkDiagnosticsEnabledAttributes & GetEnabledAttributes() const { return mEnabledAttributes; }

private:
    DeviceLayer::DiagnosticDataProvider & mDiagnosticProvider;
    const WiFiNetworkDiagnosticsEnabledAttributes mEnabledAttributes;
    const BitFlags<WiFiNetworkDiagnostics::Feature> mFeatureFlags;
};

} // namespace Clusters
} // namespace app
} // namespace chip
