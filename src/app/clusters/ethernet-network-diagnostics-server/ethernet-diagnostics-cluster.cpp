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

namespace {

CHIP_ERROR ReadPHYRate(DeviceLayer::DiagnosticDataProvider & provider, AttributeValueEncoder & encoder)
{
    EthernetNetworkDiagnostics::Attributes::PHYRate::TypeInfo::Type pHYRate;
    auto value = EthernetNetworkDiagnostics::PHYRateEnum::kRate10M;

    if (provider.GetEthPHYRate(value) == CHIP_NO_ERROR)
    {
        pHYRate.SetNonNull(value);
    }

    return encoder.Encode(pHYRate);
}

CHIP_ERROR ReadFullDuplex(DeviceLayer::DiagnosticDataProvider & provider, AttributeValueEncoder & encoder)
{
    EthernetNetworkDiagnostics::Attributes::FullDuplex::TypeInfo::Type fullDuplex;
    bool value = false;

    if (provider.GetEthFullDuplex(value) == CHIP_NO_ERROR)
    {
        fullDuplex.SetNonNull(value);
    }
    else
    {
        ChipLogProgress(Zcl, "The Ethernet interface is not currently configured or operational");
    }

    return encoder.Encode(fullDuplex);
}

CHIP_ERROR ReadCarrierDetect(DeviceLayer::DiagnosticDataProvider & provider, AttributeValueEncoder & encoder)
{
    EthernetNetworkDiagnostics::Attributes::CarrierDetect::TypeInfo::Type carrierDetect;
    bool value = false;

    if (provider.GetEthCarrierDetect(value) == CHIP_NO_ERROR)
    {
        carrierDetect.SetNonNull(value);
    }
    else
    {
        ChipLogProgress(Zcl, "The Ethernet interface is not currently configured or operational");
    }

    return encoder.Encode(carrierDetect);
}

constexpr BitFlags<EthernetNetworkDiagnostics::Feature>
GetFeatureMap(const EthernetDiagnosticsEnabledAttributes & enabledAttributes)
{
    return BitFlags<EthernetNetworkDiagnostics::Feature>()
        .Set(EthernetNetworkDiagnostics::Feature::kPacketCounts, enabledAttributes.enablePacketCount)
        .Set(EthernetNetworkDiagnostics::Feature::kErrorCounts, enabledAttributes.enableErrCount);
}

CHIP_ERROR EncodeU64Value(uint64_t value, CHIP_ERROR readError, AttributeValueEncoder & encoder)
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

} // anonymous namespace

EthernetDiagnosticsServerCluster::EthernetDiagnosticsServerCluster(DeviceLayer::DiagnosticDataProvider & provider,
                                                                   const EthernetDiagnosticsEnabledAttributes & enabledAttributes) :
    DefaultServerCluster({ kRootEndpointId, EthernetNetworkDiagnostics::Id }),
    mProvider(provider), mEnabledAttributes(enabledAttributes)
{}

DataModel::ActionReturnStatus EthernetDiagnosticsServerCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                              AttributeValueEncoder & encoder)
{
    // Handle numeric attributes inline
    uint64_t value;
    CHIP_ERROR err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;

    switch (request.path.mAttributeId)
    {
    case EthernetNetworkDiagnostics::Attributes::PHYRate::Id:
        return ReadPHYRate(mProvider, encoder);
    case EthernetNetworkDiagnostics::Attributes::FullDuplex::Id:
        return ReadFullDuplex(mProvider, encoder);
    case EthernetNetworkDiagnostics::Attributes::CarrierDetect::Id:
        return ReadCarrierDetect(mProvider, encoder);
    case EthernetNetworkDiagnostics::Attributes::TimeSinceReset::Id:
        err = mProvider.GetEthTimeSinceReset(value);
        break;
    case EthernetNetworkDiagnostics::Attributes::PacketRxCount::Id:
        err = mProvider.GetEthPacketRxCount(value);
        break;
    case EthernetNetworkDiagnostics::Attributes::PacketTxCount::Id:
        err = mProvider.GetEthPacketTxCount(value);
        break;
    case EthernetNetworkDiagnostics::Attributes::TxErrCount::Id:
        err = mProvider.GetEthTxErrCount(value);
        break;
    case EthernetNetworkDiagnostics::Attributes::CollisionCount::Id:
        err = mProvider.GetEthCollisionCount(value);
        break;
    case EthernetNetworkDiagnostics::Attributes::OverrunCount::Id:
        err = mProvider.GetEthOverrunCount(value);
        break;
    case Globals::Attributes::FeatureMap::Id:
        return encoder.Encode(GetFeatureMap(mEnabledAttributes).Raw());
    case Globals::Attributes::ClusterRevision::Id:
        return encoder.Encode(EthernetNetworkDiagnostics::kRevision);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }

    return EncodeU64Value(value, err, encoder);
}

std::optional<DataModel::ActionReturnStatus>
EthernetDiagnosticsServerCluster::InvokeCommand(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case EthernetNetworkDiagnostics::Commands::ResetCounts::Id:
        // TODO:[#39725] This should properly handle the return value from ResetEthNetworkDiagnosticsCounts().
        // However, for bug-for-bug compatibility with old code, we ignore the return value.
        // The old implementation would drop the error to the floor and always return success,
        // even when the provider returns CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE.
        (void) mProvider.ResetEthNetworkDiagnosticsCounts();
        return Protocols::InteractionModel::Status::Success;
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

CHIP_ERROR EthernetDiagnosticsServerCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                              ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    BitFlags<EthernetNetworkDiagnostics::Feature> featureMap = GetFeatureMap(mEnabledAttributes);

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

} // namespace Clusters
} // namespace app
} // namespace chip
