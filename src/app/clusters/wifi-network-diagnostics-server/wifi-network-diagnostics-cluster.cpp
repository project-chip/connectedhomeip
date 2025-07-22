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
#include <clusters/WiFiNetworkDiagnostics/Attributes.h>
#include <clusters/WiFiNetworkDiagnostics/Commands.h>
#include <clusters/WiFiNetworkDiagnostics/Ids.h>
#include <clusters/WiFiNetworkDiagnostics/Metadata.h>
#include <lib/support/CodeUtils.h>

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
    case FeatureMap::Id: {
        return encoder.Encode(mLogic.GetFeatureFlags());
    }
    case ClusterRevision::Id: {
        return encoder.Encode(WiFiNetworkDiagnostics::kRevision);
    }
    case Bssid::Id: {
        return mLogic.ReadWiFiBssId(encoder);
    }
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
        if (mLogic.GetEnabledAttributes().enableCurrentMaxRate)
        {
            Attributes::CurrentMaxRate::TypeInfo::Type rate;
            return mLogic.ReadIfSupported(&DiagnosticDataProvider::GetWiFiCurrentMaxRate, rate, encoder);
        }
        break;
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

CHIP_ERROR WiFiDiagnosticsServerCluster::Attributes(const ConcreteClusterPath & path,
                                                    ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    ReturnErrorOnFailure(builder.EnsureAppendCapacity(13 + DefaultServerCluster::GlobalAttributes().size()));

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

CHIP_ERROR WiFiDiagnosticsServerCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                          ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    ReturnErrorOnFailure(builder.EnsureAppendCapacity(1));
    if (mLogic.GetFeatureFlags().Has(Feature::kErrorCounts))
    {
        ReturnErrorOnFailure(builder.AppendElements({
            Commands::ResetCounts::kMetadataEntry,
        }));
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
        if (mLogic.GetFeatureFlags().Has(Feature::kErrorCounts))
        {
            mLogic.HandleResetCounts();
            return Protocols::InteractionModel::Status::Success;
        }
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
