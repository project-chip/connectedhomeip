/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "wifi-network-diagnostics-server.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>
#include <app/CommandHandlerInterface.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteCommandPath.h>
#include <app/EventLogging.h>
#include <app/util/attribute-storage.h>
#include <lib/core/Optional.h>
#include <tracing/macros.h>
#include <tracing/metric_event.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WiFiNetworkDiagnostics;
using namespace chip::app::Clusters::WiFiNetworkDiagnostics::Attributes;
using chip::DeviceLayer::DiagnosticDataProvider;
using chip::DeviceLayer::GetDiagnosticDataProvider;

namespace {

class WiFiDiagosticsGlobalInstance : public AttributeAccessInterface, public CommandHandlerInterface
{
public:
    // Register for the WiFiNetworkDiagnostics cluster on all endpoints.
    WiFiDiagosticsGlobalInstance(DiagnosticDataProvider & diagnosticProvider) :
        AttributeAccessInterface(Optional<EndpointId>::Missing(), WiFiNetworkDiagnostics::Id),
        CommandHandlerInterface(Optional<EndpointId>::Missing(), WiFiNetworkDiagnostics::Id),
        mDiagnosticProvider(diagnosticProvider)
    {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    template <typename T, typename Type>
    CHIP_ERROR ReadIfSupported(CHIP_ERROR (DiagnosticDataProvider::*getter)(T &), Type & data, AttributeValueEncoder & aEncoder);

    CHIP_ERROR ReadWiFiBssId(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadSecurityType(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadWiFiVersion(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadChannelNumber(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadWiFiRssi(AttributeValueEncoder & aEncoder);

    void InvokeCommand(HandlerContext & ctx) override;

#ifdef WI_FI_NETWORK_DIAGNOSTICS_ENABLE_RESET_COUNTS_CMD
    void HandleResetCounts(HandlerContext & ctx, const Commands::ResetCounts::DecodableType & commandData);
#endif

    DiagnosticDataProvider & mDiagnosticProvider;
};

template <typename T, typename Type>
CHIP_ERROR WiFiDiagosticsGlobalInstance::ReadIfSupported(CHIP_ERROR (DiagnosticDataProvider::*getter)(T &), Type & data,
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

CHIP_ERROR WiFiDiagosticsGlobalInstance::ReadWiFiBssId(AttributeValueEncoder & aEncoder)
{
    Attributes::Bssid::TypeInfo::Type bssid;

    uint8_t bssidBytes[chip::DeviceLayer::kMaxHardwareAddrSize];
    MutableByteSpan bssidSpan(bssidBytes);
    if (mDiagnosticProvider.GetWiFiBssId(bssidSpan) == CHIP_NO_ERROR)
    {
        if (!bssidSpan.empty())
        {
            bssid.SetNonNull(bssidSpan);
            ChipLogProgress(Zcl, "Node is currently connected to Wi-Fi network with BSSID:");
            ChipLogByteSpan(Zcl, bssidSpan);
        }
    }
    else
    {
        ChipLogProgress(Zcl, "The WiFi interface is not currently connected.");
    }

    return aEncoder.Encode(bssid);
}

CHIP_ERROR WiFiDiagosticsGlobalInstance::ReadSecurityType(AttributeValueEncoder & aEncoder)
{
    Attributes::SecurityType::TypeInfo::Type securityType;
    SecurityTypeEnum value = SecurityTypeEnum::kUnspecified;

    if (mDiagnosticProvider.GetWiFiSecurityType(value) == CHIP_NO_ERROR)
    {
        securityType.SetNonNull(value);
        ChipLogProgress(Zcl, "The current type of Wi-Fi security used: %d", to_underlying(value));
    }
    else
    {
        ChipLogProgress(Zcl, "The WiFi interface is not currently configured or operational.");
    }

    return aEncoder.Encode(securityType);
}

CHIP_ERROR WiFiDiagosticsGlobalInstance::ReadWiFiVersion(AttributeValueEncoder & aEncoder)
{
    Attributes::WiFiVersion::TypeInfo::Type version;
    WiFiVersionEnum value = WiFiVersionEnum::kUnknownEnumValue;

    if (mDiagnosticProvider.GetWiFiVersion(value) == CHIP_NO_ERROR)
    {
        version.SetNonNull(value);
        ChipLogProgress(Zcl, "The current 802.11 standard version in use by the Node: %d", to_underlying(value));
    }
    else
    {
        ChipLogProgress(Zcl, "The current 802.11 standard version in use by the Node is not available");
    }

    return aEncoder.Encode(version);
}

CHIP_ERROR WiFiDiagosticsGlobalInstance::ReadChannelNumber(AttributeValueEncoder & aEncoder)
{
    Attributes::ChannelNumber::TypeInfo::Type channelNumber;
    uint16_t value = 0;

    if (mDiagnosticProvider.GetWiFiChannelNumber(value) == CHIP_NO_ERROR)
    {
        channelNumber.SetNonNull(value);
        ChipLogProgress(Zcl, "The channel that Wi-Fi communication is currently operating on is: %d", value);
    }
    else
    {
        ChipLogProgress(Zcl, "The WiFi interface is not currently configured or operational.");
    }

    return aEncoder.Encode(channelNumber);
}

CHIP_ERROR WiFiDiagosticsGlobalInstance::ReadWiFiRssi(AttributeValueEncoder & aEncoder)
{
    Attributes::Rssi::TypeInfo::Type rssi;
    int8_t value = 0;

    if (mDiagnosticProvider.GetWiFiRssi(value) == CHIP_NO_ERROR)
    {
        rssi.SetNonNull(value);
        ChipLogProgress(Zcl, "The current RSSI of the Node’s Wi-Fi radio in dB: %d", value);
        MATTER_LOG_METRIC(chip::Tracing::kMetricWiFiRSSI, value);
    }
    else
    {
        ChipLogProgress(Zcl, "The WiFi interface is not currently configured or operational.");
    }

    return aEncoder.Encode(rssi);
}

CHIP_ERROR WiFiDiagosticsGlobalInstance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    if (aPath.mClusterId != WiFiNetworkDiagnostics::Id)
    {
        // We shouldn't have been called at all.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    switch (aPath.mAttributeId)
    {
    case Bssid::Id: {
        return ReadWiFiBssId(aEncoder);
    }
    case Attributes::SecurityType::Id: {
        return ReadSecurityType(aEncoder);
    }
    case WiFiVersion::Id: {
        return ReadWiFiVersion(aEncoder);
    }
    case ChannelNumber::Id: {
        return ReadChannelNumber(aEncoder);
    }
    case Rssi::Id: {
        return ReadWiFiRssi(aEncoder);
    }
    case BeaconLostCount::Id: {
        Attributes::BeaconLostCount::TypeInfo::Type count;
        return ReadIfSupported(&DiagnosticDataProvider::GetWiFiBeaconLostCount, count, aEncoder);
    }
    case BeaconRxCount::Id: {
        Attributes::BeaconRxCount::TypeInfo::Type count;
        return ReadIfSupported(&DiagnosticDataProvider::GetWiFiBeaconRxCount, count, aEncoder);
    }
    case PacketMulticastRxCount::Id: {
        Attributes::PacketMulticastRxCount::TypeInfo::Type count;
        return ReadIfSupported(&DiagnosticDataProvider::GetWiFiPacketMulticastRxCount, count, aEncoder);
    }
    case PacketMulticastTxCount::Id: {
        Attributes::PacketMulticastTxCount::TypeInfo::Type count;
        return ReadIfSupported(&DiagnosticDataProvider::GetWiFiPacketMulticastTxCount, count, aEncoder);
    }
    case PacketUnicastRxCount::Id: {
        Attributes::PacketUnicastRxCount::TypeInfo::Type count;
        return ReadIfSupported(&DiagnosticDataProvider::GetWiFiPacketUnicastRxCount, count, aEncoder);
    }
    case PacketUnicastTxCount::Id: {
        Attributes::PacketUnicastTxCount::TypeInfo::Type count;
        return ReadIfSupported(&DiagnosticDataProvider::GetWiFiPacketUnicastTxCount, count, aEncoder);
    }
    case CurrentMaxRate::Id: {
        Attributes::CurrentMaxRate::TypeInfo::Type rate;
        return ReadIfSupported(&DiagnosticDataProvider::GetWiFiCurrentMaxRate, rate, aEncoder);
    }
    case OverrunCount::Id: {
        Attributes::OverrunCount::TypeInfo::Type count;
        return ReadIfSupported(&DiagnosticDataProvider::GetWiFiOverrunCount, count, aEncoder);
    }
    default: {
        break;
    }
    }
    return CHIP_NO_ERROR;
}

void WiFiDiagosticsGlobalInstance::InvokeCommand(HandlerContext & handlerContext)
{
    switch (handlerContext.mRequestPath.mCommandId)
    {
#ifdef WI_FI_NETWORK_DIAGNOSTICS_ENABLE_RESET_COUNTS_CMD
    case Commands::ResetCounts::Id:
        CommandHandlerInterface::HandleCommand<Commands::ResetCounts::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleResetCounts(ctx, commandData); });
        break;
#endif
    }
}

#ifdef WI_FI_NETWORK_DIAGNOSTICS_ENABLE_RESET_COUNTS_CMD
void WiFiDiagosticsGlobalInstance::HandleResetCounts(HandlerContext & ctx, const Commands::ResetCounts::DecodableType & commandData)
{
    mDiagnosticProvider.ResetWiFiNetworkDiagnosticsCounts();
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
}
#endif

WiFiDiagosticsGlobalInstance gWiFiDiagosticsInstance(DeviceLayer::GetDiagnosticDataProvider());

} // anonymous namespace

namespace chip {
namespace app {
namespace Clusters {

WiFiDiagnosticsServer WiFiDiagnosticsServer::instance;

/**********************************************************
 * WiFiDiagnosticsServer Implementation
 *********************************************************/

WiFiDiagnosticsServer & WiFiDiagnosticsServer::Instance()
{
    return instance;
}

void WiFiDiagnosticsServer::OnDisconnectionDetected(uint16_t reasonCode)
{
    MATTER_TRACE_SCOPE("OnDisconnectionDetected", "WiFiDiagnosticsDelegate");
    ChipLogProgress(Zcl, "WiFiDiagnosticsDelegate: OnDisconnectionDetected");

    for (auto endpoint : EnabledEndpointsWithServerCluster(WiFiNetworkDiagnostics::Id))
    {
        // If Wi-Fi Network Diagnostics cluster is implemented on this endpoint
        Events::Disconnection::Type event{ reasonCode };
        EventNumber eventNumber;

        if (CHIP_NO_ERROR != LogEvent(event, endpoint, eventNumber))
        {
            ChipLogError(Zcl, "WiFiDiagnosticsDelegate: Failed to record Disconnection event");
        }
    }
}

void WiFiDiagnosticsServer::OnAssociationFailureDetected(uint8_t associationFailureCause, uint16_t status)
{
    MATTER_TRACE_SCOPE("OnAssociationFailureDetected", "WiFiDiagnosticsDelegate");
    ChipLogProgress(Zcl, "WiFiDiagnosticsDelegate: OnAssociationFailureDetected");

    Events::AssociationFailure::Type event{ static_cast<AssociationFailureCauseEnum>(associationFailureCause), status };

    for (auto endpoint : EnabledEndpointsWithServerCluster(WiFiNetworkDiagnostics::Id))
    {
        // If Wi-Fi Network Diagnostics cluster is implemented on this endpoint
        EventNumber eventNumber;

        if (CHIP_NO_ERROR != LogEvent(event, endpoint, eventNumber))
        {
            ChipLogError(Zcl, "WiFiDiagnosticsDelegate: Failed to record AssociationFailure event");
        }
    }
}

void WiFiDiagnosticsServer::OnConnectionStatusChanged(uint8_t connectionStatus)
{
    MATTER_TRACE_SCOPE("OnConnectionStatusChanged", "WiFiDiagnosticsDelegate");
    ChipLogProgress(Zcl, "WiFiDiagnosticsDelegate: OnConnectionStatusChanged");

    Events::ConnectionStatus::Type event{ static_cast<ConnectionStatusEnum>(connectionStatus) };
    for (auto endpoint : EnabledEndpointsWithServerCluster(WiFiNetworkDiagnostics::Id))
    {
        // If Wi-Fi Network Diagnostics cluster is implemented on this endpoint
        EventNumber eventNumber;

        if (CHIP_NO_ERROR != LogEvent(event, endpoint, eventNumber))
        {
            ChipLogError(Zcl, "WiFiDiagnosticsDelegate: Failed to record ConnectionStatus event");
        }
    }
}

} // namespace Clusters
} // namespace app
} // namespace chip

void MatterWiFiNetworkDiagnosticsPluginServerInitCallback()
{
    AttributeAccessInterfaceRegistry::Instance().Register(&gWiFiDiagosticsInstance);
    CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(&gWiFiDiagosticsInstance);

    GetDiagnosticDataProvider().SetWiFiDiagnosticsDelegate(&WiFiDiagnosticsServer::Instance());
}
