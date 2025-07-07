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

#include <app/EventLogging.h>
#include <clusters/WiFiNetworkDiagnostics/Attributes.h>
#include <clusters/WiFiNetworkDiagnostics/Commands.h>
#include <clusters/WiFiNetworkDiagnostics/Ids.h>
#include <clusters/WiFiNetworkDiagnostics/Metadata.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <tracing/macros.h>
#include <tracing/metric_event.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WiFiNetworkDiagnostics;
using namespace chip::app::Clusters::WiFiNetworkDiagnostics::Attributes;
using chip::DeviceLayer::DiagnosticDataProvider;
using chip::DeviceLayer::GetDiagnosticDataProvider;

namespace chip {
namespace app {
namespace Clusters {

template <typename T, typename Type>
CHIP_ERROR WiFiDiagnosticsServerLogic::ReadIfSupported(CHIP_ERROR (DeviceLayer::DiagnosticDataProvider::*getter)(T &), Type & data,
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

CHIP_ERROR WiFiDiagnosticsServerLogic::ReadSecurityType(AttributeValueEncoder & aEncoder)
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

CHIP_ERROR WiFiDiagnosticsServerLogic::ReadWiFiVersion(AttributeValueEncoder & aEncoder)
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

CHIP_ERROR WiFiDiagnosticsServerLogic::ReadChannelNumber(AttributeValueEncoder & aEncoder)
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

CHIP_ERROR WiFiDiagnosticsServerLogic::ReadWiFiRssi(AttributeValueEncoder & aEncoder)
{
    Attributes::Rssi::TypeInfo::Type rssi;
    int8_t value = 0;

    if (mDiagnosticProvider.GetWiFiRssi(value) == CHIP_NO_ERROR)
    {
        rssi.SetNonNull(value);
        ChipLogProgress(Zcl, "The current RSSI of the Node's Wi-Fi radio in dB: %d", value);
        MATTER_LOG_METRIC(chip::Tracing::kMetricWiFiRSSI, value);
    }
    else
    {
        ChipLogProgress(Zcl, "The WiFi interface is not currently configured or operational.");
    }

    return aEncoder.Encode(rssi);
}

void WiFiDiagnosticsServerLogic::OnDisconnectionDetected(uint16_t reasonCode)
{
    MATTER_TRACE_SCOPE("OnDisconnectionDetected", "WiFiDiagnosticsDelegate");
    ChipLogProgress(Zcl, "WiFiDiagnosticsDelegate: OnDisconnectionDetected");

    Events::Disconnection::Type event{ reasonCode };
    EventNumber eventNumber;

    if (CHIP_NO_ERROR != LogEvent(event, kRootEndpointId, eventNumber))
    {
        ChipLogError(Zcl, "WiFiDiagnosticsDelegate: Failed to record Disconnection event");
    }
}

void WiFiDiagnosticsServerLogic::OnAssociationFailureDetected(uint8_t associationFailureCause, uint16_t status)
{
    MATTER_TRACE_SCOPE("OnAssociationFailureDetected", "WiFiDiagnosticsDelegate");
    ChipLogProgress(Zcl, "WiFiDiagnosticsDelegate: OnAssociationFailureDetected");

    Events::AssociationFailure::Type event{ static_cast<AssociationFailureCauseEnum>(associationFailureCause), status };

    EventNumber eventNumber;

    if (CHIP_NO_ERROR != LogEvent(event, kRootEndpointId, eventNumber))
    {
        ChipLogError(Zcl, "WiFiDiagnosticsDelegate: Failed to record AssociationFailure event");
    }
}

void WiFiDiagnosticsServerLogic::OnConnectionStatusChanged(uint8_t connectionStatus)
{
    MATTER_TRACE_SCOPE("OnConnectionStatusChanged", "WiFiDiagnosticsDelegate");
    ChipLogProgress(Zcl, "WiFiDiagnosticsDelegate: OnConnectionStatusChanged");

    Events::ConnectionStatus::Type event{ static_cast<ConnectionStatusEnum>(connectionStatus) };

    EventNumber eventNumber;

    if (CHIP_NO_ERROR != LogEvent(event, kRootEndpointId, eventNumber))
    {
        ChipLogError(Zcl, "WiFiDiagnosticsDelegate: Failed to record ConnectionStatus event");
    }
}

DataModel::ActionReturnStatus WiFiDiagnosticsServer::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                   AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case FeatureMap::Id: {
        return encoder.Encode(mLogic.GetFeatureFlags().Raw());
    }
    case ClusterRevision::Id: {
        return encoder.Encode(WiFiNetworkDiagnostics::kRevision);
    }
    case Bssid::Id: {
        return mLogic.ReadWiFiBssId(encoder);
    }
    case Attributes::SecurityType::Id: {
        return mLogic.ReadSecurityType(encoder);
    }
    case WiFiVersion::Id: {
        return mLogic.ReadWiFiVersion(encoder);
    }
    case ChannelNumber::Id: {
        return mLogic.ReadChannelNumber(encoder);
    }
    case Rssi::Id: {
        return mLogic.ReadWiFiRssi(encoder);
    }
    case BeaconLostCount::Id: {
        if (mLogic.GetFeatureFlags().Has(Feature::kErrorCounts))
        {
            Attributes::BeaconLostCount::TypeInfo::Type count;
            return mLogic.ReadIfSupported(&DiagnosticDataProvider::GetWiFiBeaconLostCount, count, encoder);
        }
        break;
    }
    case BeaconRxCount::Id: {
        if (mLogic.GetFeatureFlags().Has(Feature::kPacketCounts))
        {
            Attributes::BeaconRxCount::TypeInfo::Type count;
            return mLogic.ReadIfSupported(&DiagnosticDataProvider::GetWiFiBeaconRxCount, count, encoder);
        }
        break;
    }
    case PacketMulticastRxCount::Id: {
        if (mLogic.GetFeatureFlags().Has(Feature::kPacketCounts))
        {
            Attributes::PacketMulticastRxCount::TypeInfo::Type count;
            return mLogic.ReadIfSupported(&DiagnosticDataProvider::GetWiFiPacketMulticastRxCount, count, encoder);
        }
        break;
    }
    case PacketMulticastTxCount::Id: {
        if (mLogic.GetFeatureFlags().Has(Feature::kPacketCounts))
        {
            Attributes::PacketMulticastTxCount::TypeInfo::Type count;
            return mLogic.ReadIfSupported(&DiagnosticDataProvider::GetWiFiPacketMulticastTxCount, count, encoder);
        }
        break;
    }
    case PacketUnicastRxCount::Id: {
        if (mLogic.GetFeatureFlags().Has(Feature::kPacketCounts))
        {
            Attributes::PacketUnicastRxCount::TypeInfo::Type count;
            return mLogic.ReadIfSupported(&DiagnosticDataProvider::GetWiFiPacketUnicastRxCount, count, encoder);
        }
        break;
    }
    case PacketUnicastTxCount::Id: {
        if (mLogic.GetFeatureFlags().Has(Feature::kPacketCounts))
        {
            Attributes::PacketUnicastTxCount::TypeInfo::Type count;
            return mLogic.ReadIfSupported(&DiagnosticDataProvider::GetWiFiPacketUnicastTxCount, count, encoder);
        }
        break;
    }
    case CurrentMaxRate::Id: {
        Attributes::CurrentMaxRate::TypeInfo::Type rate;
        return mLogic.ReadIfSupported(&DiagnosticDataProvider::GetWiFiCurrentMaxRate, rate, encoder);
    }
    case OverrunCount::Id: {
        if (mLogic.GetFeatureFlags().Has(Feature::kErrorCounts))
        {
            Attributes::OverrunCount::TypeInfo::Type count;
            return mLogic.ReadIfSupported(&DiagnosticDataProvider::GetWiFiOverrunCount, count, encoder);
        }
        break;
    }
    default:
        break;
    }
    return Protocols::InteractionModel::Status::UnsupportedAttribute;
}

CHIP_ERROR WiFiDiagnosticsServer::Attributes(const ConcreteClusterPath & path,
                                             ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    // mandatory attributes
    ReturnErrorOnFailure(builder.AppendElements({
        Bssid::kMetadataEntry,
        SecurityType::kMetadataEntry,
        WiFiVersion::kMetadataEntry,
        ChannelNumber::kMetadataEntry,
        Rssi::kMetadataEntry,
    }));

    if (mLogic.GetFeatureFlags().Has(Feature::kErrorCounts))
    {
        ReturnErrorOnFailure(builder.AppendElements({
            BeaconLostCount::kMetadataEntry,
            OverrunCount::kMetadataEntry,
        }));
    }
    if (mLogic.GetFeatureFlags().Has(Feature::kPacketCounts))
    {
        ReturnErrorOnFailure(builder.AppendElements({
            BeaconRxCount::kMetadataEntry,
            PacketMulticastRxCount::kMetadataEntry,
            PacketMulticastTxCount::kMetadataEntry,
            PacketUnicastRxCount::kMetadataEntry,
            PacketUnicastTxCount::kMetadataEntry,
        }));
    }
    if (mLogic.GetEnabledAttributes().enableCurrentMaxRate)
    {
        ReturnErrorOnFailure(builder.AppendElements({
            CurrentMaxRate::kMetadataEntry,
        }));
    }

    return builder.AppendElements(DefaultServerCluster::GlobalAttributes());
}

CHIP_ERROR WiFiDiagnosticsServer::AcceptedCommands(const ConcreteClusterPath & path,
                                                   ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    if (mLogic.GetFeatureFlags().Has(Feature::kErrorCounts))
    {
        ReturnErrorOnFailure(builder.AppendElements({
            Commands::ResetCounts::kMetadataEntry,
        }));
    }
    return CHIP_NO_ERROR;
}

std::optional<DataModel::ActionReturnStatus> WiFiDiagnosticsServer::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                  TLV::TLVReader & input_arguments,
                                                                                  CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Commands::ResetCounts::Id: {
#ifdef WI_FI_NETWORK_DIAGNOSTICS_ENABLE_RESET_COUNTS_CMD
        if (mLogic.GetFeatureFlags().Has(Feature::kErrorCounts))
        {
            mLogic.HandleResetCounts();
            return Protocols::InteractionModel::Status::Success;
        }
#endif
        break;
    }
    default:
        break;
    }
    return Protocols::InteractionModel::Status::UnsupportedCommand;
}

} // namespace Clusters
} // namespace app
} // namespace chip
