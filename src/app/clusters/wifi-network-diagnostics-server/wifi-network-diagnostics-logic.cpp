/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/EventLogging.h>
#include <app/clusters/wifi-network-diagnostics-server/wifi-network-diagnostics-logic.h>
#include <clusters/WiFiNetworkDiagnostics/Attributes.h>
#include <tracing/macros.h>
#include <tracing/metric_event.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WiFiNetworkDiagnostics;
using chip::DeviceLayer::DiagnosticDataProvider;
using chip::DeviceLayer::GetDiagnosticDataProvider;

namespace chip {
namespace app {
namespace Clusters {

CHIP_ERROR WiFiDiagnosticsServerLogic::ReadWiFiBssId(AttributeValueEncoder & aEncoder)
{
    Attributes::Bssid::TypeInfo::Type bssid;

    uint8_t bssidBytes[chip::DeviceLayer::kMaxHardwareAddrSize];
    MutableByteSpan bssidSpan(bssidBytes);
    if (mDiagnosticProvider.GetWiFiBssId(bssidSpan) == CHIP_NO_ERROR)
    {
        if (!bssidSpan.empty())
        {
            bssid.SetNonNull(bssidSpan);
        }
    }
    else
    {
        ChipLogProgress(Zcl, "The WiFi interface is not currently connected.");
    }

    return aEncoder.Encode(bssid);
}

void WiFiDiagnosticsServerLogic::OnDisconnectionDetected(uint16_t reasonCode)
{
    MATTER_TRACE_SCOPE("OnDisconnectionDetected", "WiFiDiagnosticsDelegate");

    Events::Disconnection::Type event{ reasonCode };
    EventNumber eventNumber;

    EndpointId endpointId = GetEndpointId();
    if (CHIP_NO_ERROR != LogEvent(event, endpointId, eventNumber))
    {
        ChipLogError(Zcl, "WiFiDiagnosticsDelegate: Failed to record Disconnection event");
    }
}

void WiFiDiagnosticsServerLogic::OnAssociationFailureDetected(uint8_t associationFailureCause, uint16_t status)
{
    MATTER_TRACE_SCOPE("OnAssociationFailureDetected", "WiFiDiagnosticsDelegate");

    Events::AssociationFailure::Type event{ static_cast<AssociationFailureCauseEnum>(associationFailureCause), status };
    EventNumber eventNumber;

    EndpointId endpointId = GetEndpointId();
    if (CHIP_NO_ERROR != LogEvent(event, endpointId, eventNumber))
    {
        ChipLogError(Zcl, "WiFiDiagnosticsDelegate: Failed to record AssociationFailure event");
    }
}

void WiFiDiagnosticsServerLogic::OnConnectionStatusChanged(uint8_t connectionStatus)
{
    MATTER_TRACE_SCOPE("OnConnectionStatusChanged", "WiFiDiagnosticsDelegate");

    Events::ConnectionStatus::Type event{ static_cast<ConnectionStatusEnum>(connectionStatus) };
    EventNumber eventNumber;

    EndpointId endpointId = GetEndpointId();
    if (CHIP_NO_ERROR != LogEvent(event, endpointId, eventNumber))
    {
        ChipLogError(Zcl, "WiFiDiagnosticsDelegate: Failed to record ConnectionStatus event");
    }
}

} // namespace Clusters
} // namespace app
} // namespace chip
