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

#include <app/clusters/wifi-network-diagnostics-server/wifi-network-diagnostics-cluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/WiFiNetworkDiagnostics/Ids.h>
#include <clusters/WiFiNetworkDiagnostics/Metadata.h>

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

DataModel::ActionReturnStatus WiFiDiagnosticsServerCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                          AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case FeatureMap::Id:
        return encoder.Encode(mLogic.GetFeatureFlags());
    case ClusterRevision::Id:
        return encoder.Encode(WiFiNetworkDiagnostics::kRevision);
    case Bssid::Id:
        return mLogic.ReadWiFiBssId(encoder);
    case Attributes::SecurityType::Id: {
        Attributes::SecurityType::TypeInfo::Type securityType;
        return mLogic.ReadIfSupported(&DiagnosticDataProvider::GetWiFiSecurityType, securityType, encoder);
    }
    case WiFiVersion::Id: {
        Attributes::WiFiVersion::TypeInfo::Type version;
        return mLogic.ReadIfSupported(&DiagnosticDataProvider::GetWiFiVersion, version, encoder);
    }
    case ChannelNumber::Id: {
        Attributes::ChannelNumber::TypeInfo::Type channelNumber;
        return mLogic.ReadIfSupported(&DiagnosticDataProvider::GetWiFiChannelNumber, channelNumber, encoder);
    }
    case Rssi::Id: {
        Attributes::Rssi::TypeInfo::Type rssi;
        return mLogic.ReadIfSupported(&DiagnosticDataProvider::GetWiFiRssi, rssi, encoder);
    }
    case BeaconLostCount::Id: {
        Attributes::BeaconLostCount::TypeInfo::Type count;
        return mLogic.ReadIfSupported(&DiagnosticDataProvider::GetWiFiBeaconLostCount, count, encoder);
    }
    case BeaconRxCount::Id: {
        Attributes::BeaconRxCount::TypeInfo::Type count;
        return mLogic.ReadIfSupported(&DiagnosticDataProvider::GetWiFiBeaconRxCount, count, encoder);
    }
    case PacketMulticastRxCount::Id: {
        Attributes::PacketMulticastRxCount::TypeInfo::Type count;
        return mLogic.ReadIfSupported(&DiagnosticDataProvider::GetWiFiPacketMulticastRxCount, count, encoder);
    }
    case PacketMulticastTxCount::Id: {
        Attributes::PacketMulticastTxCount::TypeInfo::Type count;
        return mLogic.ReadIfSupported(&DiagnosticDataProvider::GetWiFiPacketMulticastTxCount, count, encoder);
    }
    case PacketUnicastRxCount::Id: {
        Attributes::PacketUnicastRxCount::TypeInfo::Type count;
        return mLogic.ReadIfSupported(&DiagnosticDataProvider::GetWiFiPacketUnicastRxCount, count, encoder);
    }
    case PacketUnicastTxCount::Id: {
        Attributes::PacketUnicastTxCount::TypeInfo::Type count;
        return mLogic.ReadIfSupported(&DiagnosticDataProvider::GetWiFiPacketUnicastTxCount, count, encoder);
    }
    case CurrentMaxRate::Id: {
        Attributes::CurrentMaxRate::TypeInfo::Type rate;
        return mLogic.ReadIfSupported(&DiagnosticDataProvider::GetWiFiCurrentMaxRate, rate, encoder);
    }
    case OverrunCount::Id: {
        Attributes::OverrunCount::TypeInfo::Type count;
        return mLogic.ReadIfSupported(&DiagnosticDataProvider::GetWiFiOverrunCount, count, encoder);
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR WiFiDiagnosticsServerCluster::Attributes(const ConcreteClusterPath & path,
                                                    ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder attributeListBuilder(builder);

    const DataModel::AttributeEntry mandatoryAttributes[] = {
        Bssid::kMetadataEntry,         SecurityType::kMetadataEntry, WiFiVersion::kMetadataEntry,
        ChannelNumber::kMetadataEntry, Rssi::kMetadataEntry,
    };

    const BitFlags<WiFiNetworkDiagnostics::Feature> featureFlags = mLogic.GetFeatureFlags();

    const bool hasErrorCounts  = featureFlags.Has(Feature::kErrorCounts);
    const bool hasPacketCounts = featureFlags.Has(Feature::kPacketCounts);
    // Define optional attributes based on features
    const AttributeListBuilder::OptionalAttributeEntry optionalEntries[] = {
        { hasErrorCounts, BeaconLostCount::kMetadataEntry },
        { hasErrorCounts, OverrunCount::kMetadataEntry },
        { hasPacketCounts, BeaconRxCount::kMetadataEntry },
        { hasPacketCounts, PacketMulticastRxCount::kMetadataEntry },
        { hasPacketCounts, PacketMulticastTxCount::kMetadataEntry },
        { hasPacketCounts, PacketUnicastRxCount::kMetadataEntry },
        { hasPacketCounts, PacketUnicastTxCount::kMetadataEntry },
        { mLogic.GetEnabledAttributes().enableCurrentMaxRate, CurrentMaxRate::kMetadataEntry },
    };

    return attributeListBuilder.Append(Span(mandatoryAttributes), Span(optionalEntries));
}

CHIP_ERROR WiFiDiagnosticsServerCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                          ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    if (mLogic.GetFeatureFlags().Has(Feature::kErrorCounts))
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
        mLogic.HandleResetCounts();
        return Protocols::InteractionModel::Status::Success;
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

} // namespace Clusters
} // namespace app
} // namespace chip
