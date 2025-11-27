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

EthernetDiagnosticsServerCluster::EthernetDiagnosticsServerCluster(DeviceLayer::DiagnosticDataProvider & provider,
                                                                   const BitFlags<Feature> enabledFeatures,
                                                                   OptionalAttributeSet optionalAttributeSet) :
    DefaultServerCluster({ kRootEndpointId, Id }),
    mProvider(provider), mEnabledFeatures(enabledFeatures), mOptionalAttributeSet(optionalAttributeSet)
{}

DataModel::ActionReturnStatus EthernetDiagnosticsServerCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                              AttributeValueEncoder & encoder)
{
    uint64_t value;
    CHIP_ERROR err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;

    switch (request.path.mAttributeId)
    {
    case PHYRate::Id: {
        PHYRate::TypeInfo::Type pHYRate;
        auto phyRateValue = PHYRateEnum::kRate10M;

        if (mProvider.GetEthPHYRate(phyRateValue) == CHIP_NO_ERROR)
        {
            pHYRate.SetNonNull(phyRateValue);
            ChipLogProgress(Zcl, "The current nominal, usable speed at the top of the physical layer of the Node: %d",
                            chip::to_underlying(phyRateValue));
        }
        else
        {
            ChipLogProgress(Zcl, "The Ethernet interface is not currently configured or operational");
        }

        return encoder.Encode(pHYRate);
    }
    case FullDuplex::Id: {
        FullDuplex::TypeInfo::Type fullDuplex;
        bool duplexValue = false;

        if (mProvider.GetEthFullDuplex(duplexValue) == CHIP_NO_ERROR)
        {
            fullDuplex.SetNonNull(duplexValue);
            ChipLogProgress(Zcl, "The full-duplex operating status of Node: %d", duplexValue);
        }
        else
        {
            ChipLogProgress(Zcl, "The Ethernet interface is not currently configured or operational");
        }

        return encoder.Encode(fullDuplex);
    }
    case CarrierDetect::Id: {
        CarrierDetect::TypeInfo::Type carrierDetect;
        bool carrierDetectValue = false;

        if (mProvider.GetEthCarrierDetect(carrierDetectValue) == CHIP_NO_ERROR)
        {
            carrierDetect.SetNonNull(carrierDetectValue);
            ChipLogProgress(Zcl, "The status of the Carrier Detect control signal present on the ethernet network interface: %d",
                            carrierDetectValue);
        }
        else
        {
            ChipLogProgress(Zcl, "The Ethernet interface is not currently configured or operational");
        }

        return encoder.Encode(carrierDetect);
    }
    case TimeSinceReset::Id:
        err = mProvider.GetEthTimeSinceReset(value);
        break;
    case PacketRxCount::Id:
        err = mProvider.GetEthPacketRxCount(value);
        break;
    case PacketTxCount::Id:
        err = mProvider.GetEthPacketTxCount(value);
        break;
    case TxErrCount::Id:
        err = mProvider.GetEthTxErrCount(value);
        break;
    case CollisionCount::Id:
        err = mProvider.GetEthCollisionCount(value);
        break;
    case OverrunCount::Id:
        err = mProvider.GetEthOverrunCount(value);
        break;
    case Globals::Attributes::FeatureMap::Id:
        return encoder.Encode(mEnabledFeatures);
    case Globals::Attributes::ClusterRevision::Id:
        return encoder.Encode(kRevision);
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
    case Commands::ResetCounts::Id:
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
    if (mEnabledFeatures.Has(Feature::kPacketCounts) || mEnabledFeatures.Has(Feature::kErrorCounts))
    {
        static constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = { Commands::ResetCounts::kMetadataEntry };
        return builder.ReferenceExisting(kAcceptedCommands);
    }

    // no commands supported
    return CHIP_NO_ERROR;
}

CHIP_ERROR EthernetDiagnosticsServerCluster::Attributes(const ConcreteClusterPath & path,
                                                        ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    const bool havePacketCounts = mEnabledFeatures.Has(Feature::kPacketCounts);
    const bool haveErrorCounts  = mEnabledFeatures.Has(Feature::kErrorCounts);

    // Counts only via feature bits; other optionals via optional attribute set.
    AttributeListBuilder::OptionalAttributeEntry optionalAttributeEntries[] = {
        { havePacketCounts, PacketRxCount::kMetadataEntry },
        { havePacketCounts, PacketTxCount::kMetadataEntry },
        { haveErrorCounts, TxErrCount::kMetadataEntry },
        { haveErrorCounts, CollisionCount::kMetadataEntry },
        { haveErrorCounts, OverrunCount::kMetadataEntry },
        { mOptionalAttributeSet.IsSet(CarrierDetect::Id), CarrierDetect::kMetadataEntry },
        { mOptionalAttributeSet.IsSet(FullDuplex::Id), FullDuplex::kMetadataEntry },
        { mOptionalAttributeSet.IsSet(PHYRate::Id), PHYRate::kMetadataEntry },
        { mOptionalAttributeSet.IsSet(TimeSinceReset::Id), TimeSinceReset::kMetadataEntry },
    };

    return listBuilder.Append(Span(Attributes::kMandatoryMetadata), Span(optionalAttributeEntries));
}

} // namespace Clusters
} // namespace app
} // namespace chip
