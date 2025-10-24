/**
 *
 *    Copyright (c) 2021-25 Project CHIP Authors
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

#include "lib/support/CodeUtils.h"
#include <app/EventLogging.h>
#include <app/clusters/wifi-network-diagnostics-server/wifi-network-diagnostics-cluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/WiFiNetworkDiagnostics/Ids.h>
#include <clusters/WiFiNetworkDiagnostics/Metadata.h>
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

CHIP_ERROR WiFiDiagnosticsServerCluster::ReadWiFiBssId(AttributeValueEncoder & aEncoder)
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

void WiFiDiagnosticsServerCluster::OnDisconnectionDetected(uint16_t reasonCode)
{
    MATTER_TRACE_SCOPE("OnDisconnectionDetected", "WiFiDiagnosticsDelegate");

    VerifyOrReturn(mContext != nullptr);

    Events::Disconnection::Type event{ reasonCode };
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, GetEndpointId());
}

void WiFiDiagnosticsServerCluster::OnAssociationFailureDetected(uint8_t associationFailureCause, uint16_t status)
{
    MATTER_TRACE_SCOPE("OnAssociationFailureDetected", "WiFiDiagnosticsDelegate");

    VerifyOrReturn(mContext != nullptr);

    Events::AssociationFailure::Type event{ static_cast<AssociationFailureCauseEnum>(associationFailureCause), status };
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, GetEndpointId());
}

void WiFiDiagnosticsServerCluster::OnConnectionStatusChanged(uint8_t connectionStatus)
{
    MATTER_TRACE_SCOPE("OnConnectionStatusChanged", "WiFiDiagnosticsDelegate");

    VerifyOrReturn(mContext != nullptr);

    Events::ConnectionStatus::Type event{ static_cast<ConnectionStatusEnum>(connectionStatus) };
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, GetEndpointId());
}

DataModel::ActionReturnStatus WiFiDiagnosticsServerCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                          AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case FeatureMap::Id:
        return encoder.Encode(mFeatureFlags);
    case ClusterRevision::Id:
        return encoder.Encode(WiFiNetworkDiagnostics::kRevision);
    case Bssid::Id:
        return ReadWiFiBssId(encoder);
    case Attributes::SecurityType::Id: {
        Attributes::SecurityType::TypeInfo::Type securityType;
        return ReadIfSupported(&DiagnosticDataProvider::GetWiFiSecurityType, securityType, encoder);
    }
    case WiFiVersion::Id: {
        Attributes::WiFiVersion::TypeInfo::Type version;
        return ReadIfSupported(&DiagnosticDataProvider::GetWiFiVersion, version, encoder);
    }
    case ChannelNumber::Id: {
        Attributes::ChannelNumber::TypeInfo::Type channelNumber;
        return ReadIfSupported(&DiagnosticDataProvider::GetWiFiChannelNumber, channelNumber, encoder);
    }
    case Rssi::Id: {
        Attributes::Rssi::TypeInfo::Type rssi;
        return ReadIfSupported(&DiagnosticDataProvider::GetWiFiRssi, rssi, encoder);
    }
    case BeaconLostCount::Id: {
        Attributes::BeaconLostCount::TypeInfo::Type count;
        return ReadIfSupported(&DiagnosticDataProvider::GetWiFiBeaconLostCount, count, encoder);
    }
    case BeaconRxCount::Id: {
        Attributes::BeaconRxCount::TypeInfo::Type count;
        return ReadIfSupported(&DiagnosticDataProvider::GetWiFiBeaconRxCount, count, encoder);
    }
    case PacketMulticastRxCount::Id: {
        Attributes::PacketMulticastRxCount::TypeInfo::Type count;
        return ReadIfSupported(&DiagnosticDataProvider::GetWiFiPacketMulticastRxCount, count, encoder);
    }
    case PacketMulticastTxCount::Id: {
        Attributes::PacketMulticastTxCount::TypeInfo::Type count;
        return ReadIfSupported(&DiagnosticDataProvider::GetWiFiPacketMulticastTxCount, count, encoder);
    }
    case PacketUnicastRxCount::Id: {
        Attributes::PacketUnicastRxCount::TypeInfo::Type count;
        return ReadIfSupported(&DiagnosticDataProvider::GetWiFiPacketUnicastRxCount, count, encoder);
    }
    case PacketUnicastTxCount::Id: {
        Attributes::PacketUnicastTxCount::TypeInfo::Type count;
        return ReadIfSupported(&DiagnosticDataProvider::GetWiFiPacketUnicastTxCount, count, encoder);
    }
    case CurrentMaxRate::Id: {
        Attributes::CurrentMaxRate::TypeInfo::Type rate;
        return ReadIfSupported(&DiagnosticDataProvider::GetWiFiCurrentMaxRate, rate, encoder);
    }
    case OverrunCount::Id: {
        Attributes::OverrunCount::TypeInfo::Type count;
        return ReadIfSupported(&DiagnosticDataProvider::GetWiFiOverrunCount, count, encoder);
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR WiFiDiagnosticsServerCluster::Attributes(const ConcreteClusterPath & path,
                                                    ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder attributeListBuilder(builder);

    const BitFlags<WiFiNetworkDiagnostics::Feature> featureFlags = mFeatureFlags;

    const DataModel::AttributeEntry optionalAttributes[] = {
        BeaconLostCount::kMetadataEntry,        //
        OverrunCount::kMetadataEntry,           //
        BeaconRxCount::kMetadataEntry,          //
        PacketMulticastRxCount::kMetadataEntry, //
        PacketMulticastTxCount::kMetadataEntry, //
        PacketUnicastRxCount::kMetadataEntry,   //
        PacketUnicastTxCount::kMetadataEntry,   //
        CurrentMaxRate::kMetadataEntry,         //
    };

    // Full attribute set, to combine real "optional" attributes but also
    // attributes controlled by feature flags.
    chip::app::OptionalAttributeSet<WiFiNetworkDiagnostics::Attributes::CurrentMaxRate::Id,         //
                                    WiFiNetworkDiagnostics::Attributes::BeaconLostCount::Id,        //
                                    WiFiNetworkDiagnostics::Attributes::OverrunCount::Id,           //
                                    WiFiNetworkDiagnostics::Attributes::BeaconRxCount::Id,          //
                                    WiFiNetworkDiagnostics::Attributes::PacketMulticastRxCount::Id, //
                                    WiFiNetworkDiagnostics::Attributes::PacketUnicastRxCount::Id,   //
                                    WiFiNetworkDiagnostics::Attributes::PacketMulticastTxCount::Id, //
                                    WiFiNetworkDiagnostics::Attributes::PacketUnicastTxCount::Id    //
                                    >
        optionalAttributeSet(mOptionalAttributeSet);

    if (featureFlags.Has(Feature::kErrorCounts))
    {
        optionalAttributeSet.Set<BeaconLostCount::Id>();
        optionalAttributeSet.Set<OverrunCount::Id>();
    }

    if (featureFlags.Has(Feature::kPacketCounts))
    {
        optionalAttributeSet.Set<BeaconRxCount::Id>();
        optionalAttributeSet.Set<PacketMulticastRxCount::Id>();
        optionalAttributeSet.Set<PacketMulticastTxCount::Id>();
        optionalAttributeSet.Set<PacketUnicastRxCount::Id>();
        optionalAttributeSet.Set<PacketUnicastTxCount::Id>();
    }
    return attributeListBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), optionalAttributeSet);
}

CHIP_ERROR WiFiDiagnosticsServerCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                          ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    if (mFeatureFlags.Has(Feature::kErrorCounts))
    {
        static constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = { Commands::ResetCounts::kMetadataEntry };
        return builder.ReferenceExisting(kAcceptedCommands);
    }

    return CHIP_NO_ERROR;
}

std::optional<DataModel::ActionReturnStatus> WiFiDiagnosticsServerCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                         TLV::TLVReader & input_arguments,
                                                                                         CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Commands::ResetCounts::Id: {
        // Note that this reset does return a CHIP_ERROR however for backwards compatibility
        // we completely ignore that. It would probably be more correct to return the reset
        // result, however that seems to potentially cause backwards compatibility issues.
        mDiagnosticProvider.ResetWiFiNetworkDiagnosticsCounts();
        return Protocols::InteractionModel::Status::Success;
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

} // namespace Clusters
} // namespace app
} // namespace chip
