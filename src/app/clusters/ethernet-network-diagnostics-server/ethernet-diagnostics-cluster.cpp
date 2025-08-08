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

#include <app/server-cluster/AttributeListBuilder.h>
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
using namespace chip::app::Clusters::EthernetNetworkDiagnostics::Attributes;

namespace {

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

EthernetDiagnosticsServerCluster::EthernetDiagnosticsServerCluster(
    DeviceLayer::DiagnosticDataProvider & provider, const BitFlags<EthernetNetworkDiagnostics::Feature> enabledFeatures,
    const EthernetDiagnosticsEnabledAttributes & enabledAttributes) :
    DefaultServerCluster({ kRootEndpointId, EthernetNetworkDiagnostics::Id }), mProvider(provider),
    mEnabledFeatures(enabledFeatures), mEnabledAttributes(enabledAttributes)
{}

DataModel::ActionReturnStatus EthernetDiagnosticsServerCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                              AttributeValueEncoder & encoder)
{
    // Handle numeric attributes inline
    uint64_t value;
    CHIP_ERROR err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;

    switch (request.path.mAttributeId)
    {
    case EthernetNetworkDiagnostics::Attributes::PHYRate::Id: {
        EthernetNetworkDiagnostics::Attributes::PHYRate::TypeInfo::Type pHYRate;
        auto phyRateValue = EthernetNetworkDiagnostics::PHYRateEnum::kRate10M;

        if (mProvider.GetEthPHYRate(phyRateValue) == CHIP_NO_ERROR)
        {
            pHYRate.SetNonNull(phyRateValue);
        }

        return encoder.Encode(pHYRate);
    }
    case EthernetNetworkDiagnostics::Attributes::FullDuplex::Id: {
        EthernetNetworkDiagnostics::Attributes::FullDuplex::TypeInfo::Type fullDuplex;
        bool duplexValue = false;

        if (mProvider.GetEthFullDuplex(duplexValue) == CHIP_NO_ERROR)
        {
            fullDuplex.SetNonNull(value);
        }

        return encoder.Encode(fullDuplex);
    }
    case EthernetNetworkDiagnostics::Attributes::CarrierDetect::Id: {
        EthernetNetworkDiagnostics::Attributes::CarrierDetect::TypeInfo::Type carrierDetect;
        bool carrierDetectValue = false;

        if (mProvider.GetEthCarrierDetect(carrierDetectValue) == CHIP_NO_ERROR)
        {
            carrierDetect.SetNonNull(carrierDetectValue);
        }

        return encoder.Encode(carrierDetect);
    }
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
        return encoder.Encode(mEnabledFeatures);
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
    if (mEnabledFeatures.Has(EthernetNetworkDiagnostics::Feature::kPacketCounts) ||
        mEnabledFeatures.Has(EthernetNetworkDiagnostics::Feature::kErrorCounts))
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
    bool hasErrorCounts  = mEnabledFeatures.Has(EthernetNetworkDiagnostics::Feature::kErrorCounts);
    bool hasPacketCounts = mEnabledFeatures.Has(EthernetNetworkDiagnostics::Feature::kPacketCounts);

    const AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { mEnabledAttributes.enableCarrierDetect, CarrierDetect::kMetadataEntry },
        { mEnabledAttributes.enableFullDuplex, FullDuplex::kMetadataEntry },
        { mEnabledAttributes.enablePHYRate, PHYRate::kMetadataEntry },
        { mEnabledAttributes.enableTimeSinceReset, TimeSinceReset::kMetadataEntry },
        { hasErrorCounts, OverrunCount::kMetadataEntry },
        { hasErrorCounts, CollisionCount::kMetadataEntry },
        { hasErrorCounts, TxErrCount::kMetadataEntry },
        { hasPacketCounts, PacketRxCount::kMetadataEntry },
        { hasPacketCounts, PacketTxCount::kMetadataEntry },
    };

    AttributeListBuilder listBuilder(builder);

    // Ethernet diagnostics cluster has no mandatory attributes beyond global ones
    return listBuilder.Append(Span<const DataModel::AttributeEntry>(), Span(optionalAttributes));
}

} // namespace Clusters
} // namespace app
} // namespace chip
