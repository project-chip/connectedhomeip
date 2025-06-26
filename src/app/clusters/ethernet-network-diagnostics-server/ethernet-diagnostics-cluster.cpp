/*
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
#include <app/clusters/ethernet-network-diagnostics-server/ethernet-diagnostics-cluster.h>

#include <app/server-cluster/DefaultServerCluster.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <clusters/EthernetNetworkDiagnostics/Attributes.h>
#include <clusters/EthernetNetworkDiagnostics/Commands.h>
#include <clusters/EthernetNetworkDiagnostics/Ids.h>
#include <clusters/EthernetNetworkDiagnostics/Metadata.h>

namespace chip {
namespace app {
namespace Clusters {

using namespace chip::app::Clusters::EthernetNetworkDiagnostics;

EthernetDiagnosticsServerCluster::EthernetDiagnosticsServerCluster(DeviceLayer::DiagnosticDataProvider & provider,
                                                                   const EthernetDiagnosticsEnabledAttributes & enabledAttributes) :
    DefaultServerCluster({ kRootEndpointId, EthernetNetworkDiagnostics::Id }),
    mProvider(provider), mEnabledAttributes(enabledAttributes)
{}

DataModel::ActionReturnStatus EthernetDiagnosticsServerCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                              AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case EthernetNetworkDiagnostics::Attributes::PHYRate::Id:
        return ReadPHYRate(encoder);
    case EthernetNetworkDiagnostics::Attributes::FullDuplex::Id:
        return ReadFullDuplex(encoder);
    case EthernetNetworkDiagnostics::Attributes::CarrierDetect::Id:
        return ReadCarrierDetect(encoder);
    case EthernetNetworkDiagnostics::Attributes::TimeSinceReset::Id: {
        uint64_t value;
        CHIP_ERROR err = mProvider.GetEthTimeSinceReset(value);
        return EncodeValue(value, err, encoder);
    }
    case EthernetNetworkDiagnostics::Attributes::PacketRxCount::Id: {
        uint64_t value;
        CHIP_ERROR err = mProvider.GetEthPacketRxCount(value);
        return EncodeValue(value, err, encoder);
    }
    case EthernetNetworkDiagnostics::Attributes::PacketTxCount::Id: {
        uint64_t value;
        CHIP_ERROR err = mProvider.GetEthPacketTxCount(value);
        return EncodeValue(value, err, encoder);
    }
    case EthernetNetworkDiagnostics::Attributes::TxErrCount::Id: {
        uint64_t value;
        CHIP_ERROR err = mProvider.GetEthTxErrCount(value);
        return EncodeValue(value, err, encoder);
    }
    case EthernetNetworkDiagnostics::Attributes::CollisionCount::Id: {
        uint64_t value;
        CHIP_ERROR err = mProvider.GetEthCollisionCount(value);
        return EncodeValue(value, err, encoder);
    }
    case EthernetNetworkDiagnostics::Attributes::OverrunCount::Id: {
        uint64_t value;
        CHIP_ERROR err = mProvider.GetEthOverrunCount(value);
        return EncodeValue(value, err, encoder);
    }
    case Globals::Attributes::FeatureMap::Id:
        return encoder.Encode(GetFeatureMap().Raw());
    case Globals::Attributes::ClusterRevision::Id:
        return encoder.Encode(EthernetNetworkDiagnostics::kRevision);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

std::optional<DataModel::ActionReturnStatus>
EthernetDiagnosticsServerCluster::InvokeCommand(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case EthernetNetworkDiagnostics::Commands::ResetCounts::Id:
        return mProvider.ResetEthNetworkDiagnosticsCounts();
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

CHIP_ERROR EthernetDiagnosticsServerCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                              ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    BitFlags<EthernetNetworkDiagnostics::Feature> featureMap = GetFeatureMap();

    if (featureMap.Has(EthernetNetworkDiagnostics::Feature::kPacketCounts) ||
        featureMap.Has(EthernetNetworkDiagnostics::Feature::kErrorCounts))
    {
        static constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = {
            EthernetNetworkDiagnostics::Commands::ResetCounts::kMetadataEntry
        };
        return builder.ReferenceExisting(kAcceptedCommands);
    }

    // no commands supported
    return CHIP_NO_ERROR;
}

CHIP_ERROR EthernetDiagnosticsServerCluster::Attributes(const ConcreteClusterPath & path,
                                                        ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    struct Mapping
    {
        const bool enable;
        const DataModel::AttributeEntry & entry;
    };

    const Mapping mappings[] = {
        { mEnabledAttributes.enableCarrierDetect, EthernetNetworkDiagnostics::Attributes::CarrierDetect::kMetadataEntry },
        { mEnabledAttributes.enableErrCount, EthernetNetworkDiagnostics::Attributes::CollisionCount::kMetadataEntry },
        { mEnabledAttributes.enableFullDuplex, EthernetNetworkDiagnostics::Attributes::FullDuplex::kMetadataEntry },
        { mEnabledAttributes.enableErrCount, EthernetNetworkDiagnostics::Attributes::OverrunCount::kMetadataEntry },
        { mEnabledAttributes.enablePacketCount, EthernetNetworkDiagnostics::Attributes::PacketRxCount::kMetadataEntry },
        { mEnabledAttributes.enablePacketCount, EthernetNetworkDiagnostics::Attributes::PacketTxCount::kMetadataEntry },
        { mEnabledAttributes.enablePHYRate, EthernetNetworkDiagnostics::Attributes::PHYRate::kMetadataEntry },
        { mEnabledAttributes.enableTimeSinceReset, EthernetNetworkDiagnostics::Attributes::TimeSinceReset::kMetadataEntry },
        { mEnabledAttributes.enableErrCount, EthernetNetworkDiagnostics::Attributes::TxErrCount::kMetadataEntry },
    };

    ReturnErrorOnFailure(builder.EnsureAppendCapacity(std::size(mappings) + DefaultServerCluster::GlobalAttributes().size()));

    for (const auto & m : mappings)
    {
        if (m.enable)
        {
            ReturnErrorOnFailure(builder.Append(m.entry));
        }
    }

    return builder.AppendElements(DefaultServerCluster::GlobalAttributes());
}

CHIP_ERROR EthernetDiagnosticsServerCluster::ReadPHYRate(AttributeValueEncoder & encoder) const
{
    EthernetNetworkDiagnostics::Attributes::PHYRate::TypeInfo::Type pHYRate;
    auto value = EthernetNetworkDiagnostics::PHYRateEnum::kRate10M;

    if (mProvider.GetEthPHYRate(value) == CHIP_NO_ERROR)
    {
        pHYRate.SetNonNull(value);
        ChipLogProgress(Zcl, "The current nominal, usable speed at the top of the physical layer of the Node: %d",
                        chip::to_underlying(value));
    }
    else
    {
        ChipLogProgress(Zcl, "The Ethernet interface is not currently configured or operational");
    }

    return encoder.Encode(pHYRate);
}

CHIP_ERROR EthernetDiagnosticsServerCluster::ReadFullDuplex(AttributeValueEncoder & encoder) const
{
    EthernetNetworkDiagnostics::Attributes::FullDuplex::TypeInfo::Type fullDuplex;
    bool value = false;

    if (mProvider.GetEthFullDuplex(value) == CHIP_NO_ERROR)
    {
        fullDuplex.SetNonNull(value);
        ChipLogProgress(Zcl, "The full-duplex operating status of Node: %d", value);
    }
    else
    {
        ChipLogProgress(Zcl, "The Ethernet interface is not currently configured or operational");
    }

    return encoder.Encode(fullDuplex);
}

CHIP_ERROR EthernetDiagnosticsServerCluster::ReadCarrierDetect(AttributeValueEncoder & encoder) const
{
    EthernetNetworkDiagnostics::Attributes::CarrierDetect::TypeInfo::Type carrierDetect;
    bool value = false;

    if (mProvider.GetEthCarrierDetect(value) == CHIP_NO_ERROR)
    {
        carrierDetect.SetNonNull(value);
        ChipLogProgress(Zcl, "The status of the Carrier Detect control signal present on the ethernet network interface: %d",
                        value);
    }
    else
    {
        ChipLogProgress(Zcl, "The Ethernet interface is not currently configured or operational");
    }

    return encoder.Encode(carrierDetect);
}

BitFlags<EthernetNetworkDiagnostics::Feature> EthernetDiagnosticsServerCluster::GetFeatureMap() const
{
    return BitFlags<EthernetNetworkDiagnostics::Feature>()
        .Set(EthernetNetworkDiagnostics::Feature::kPacketCounts, mEnabledAttributes.enablePacketCount)
        .Set(EthernetNetworkDiagnostics::Feature::kErrorCounts, mEnabledAttributes.enableErrCount);
}

CHIP_ERROR EthernetDiagnosticsServerCluster::EncodeValue(uint64_t value, CHIP_ERROR readError, AttributeValueEncoder & encoder)
{
    if (readError == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
    {
        value = 0;
    }
    else if (readError != CHIP_NO_ERROR)
    {
        return readError;
    }
    return encoder.Encode(value);
}

} // namespace Clusters
} // namespace app
} // namespace chip
