/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-enums.h>
#include <app/clusters/wifi-network-diagnostics-server/WiFiDiagnosticsTestEventTriggerHandler.h>
#include <app/clusters/wifi-network-diagnostics-server/wifi-network-diagnostics-server.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DiagnosticDataProvider.h>

#include <ctime>
#include <string>

using namespace chip;
using namespace chip::app;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters;

namespace {

/**
 * @brief Helper function to simulate a Disconnection event
 */
void SetTestEventTrigger_Disconnection()
{
    uint16_t reasonCode = 3; // Deauthenticated because sending STA is leaving (or has left) IBSS or ESS.

    WiFiDiagnosticsServer::Instance().OnDisconnectionDetected(reasonCode);
}

/**
 * @brief Helper function to simulate an Association Failure event
 */
void SetTestEventTrigger_AssociationFailure()
{
    uint8_t associationFailureCause =
        static_cast<uint8_t>(WiFiNetworkDiagnostics::AssociationFailureCauseEnum::kAuthenticationFailed);
    uint16_t status = 4; // IEEE 802.11-2020 Status Codes, AP is unable to handle additional associated STAs

    WiFiDiagnosticsServer::Instance().OnAssociationFailureDetected(associationFailureCause, status);
}

/**
 * @brief Helper function to simulate a Connection Status event
 */
void SetTestEventTrigger_ConnectionStatus()
{
    uint8_t connectionStatus = static_cast<uint8_t>(WiFiNetworkDiagnostics::ConnectionStatusEnum::kNotConnected);
    WiFiDiagnosticsServer::Instance().OnConnectionStatusChanged(connectionStatus);
}

} // anonymous namespace

bool HandleWiFiDiagnosticsTestEventTrigger(uint64_t eventTrigger)
{
    // Convert raw trigger to our enum
    WiFiDiagnosticsTrigger trigger = static_cast<WiFiDiagnosticsTrigger>(eventTrigger);

    switch (trigger)
    {
    case WiFiDiagnosticsTrigger::kDisconnection:
        ChipLogProgress(Support, "[WiFiDiagnostics-Test-Event] => Disconnection triggered");
        SetTestEventTrigger_Disconnection();
        break;

    case WiFiDiagnosticsTrigger::kAssociationFailure:
        ChipLogProgress(Support, "[WiFiDiagnostics-Test-Event] => AssociationFailure triggered");
        SetTestEventTrigger_AssociationFailure();
        break;

    case WiFiDiagnosticsTrigger::kConnectionStatus:
        ChipLogProgress(Support, "[WiFiDiagnostics-Test-Event] => ConnectionStatus triggered");
        SetTestEventTrigger_ConnectionStatus();
        break;

    default:
        // If we get here, the trigger value is unknown to this handler
        return false;
    }

    // Indicate that we handled the trigger successfully
    return true;
}
