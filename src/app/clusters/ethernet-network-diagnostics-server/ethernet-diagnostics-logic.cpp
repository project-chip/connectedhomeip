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
#include <app/clusters/ethernet-network-diagnostics-server/ethernet-diagnostics-logic.h>

#include <app/server-cluster/DefaultServerCluster.h>
#include <lib/support/CodeUtils.h>

#include <clusters/EthernetNetworkDiagnostics/Commands.h>
#include <clusters/EthernetNetworkDiagnostics/Ids.h>
#include <clusters/EthernetNetworkDiagnostics/Metadata.h>

namespace chip {
namespace app {
namespace Clusters {

using namespace chip::app::Clusters::EthernetNetworkDiagnostics;

CHIP_ERROR EthernetDiagnosticsLogic::ReadPHYRate(AttributeValueEncoder & encoder) const
{
    Attributes::PHYRate::TypeInfo::Type pHYRate;
    auto value = EthernetNetworkDiagnostics::PHYRateEnum::kRate10M;

    if (DeviceLayer::GetDiagnosticDataProvider().GetEthPHYRate(value) == CHIP_NO_ERROR)
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

CHIP_ERROR EthernetDiagnosticsLogic::ReadFullDuplex(AttributeValueEncoder & encoder) const
{
    Attributes::FullDuplex::TypeInfo::Type fullDuplex;
    bool value = false;

    if (GetDiagnosticDataProvider().GetEthFullDuplex(value) == CHIP_NO_ERROR)
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

CHIP_ERROR EthernetDiagnosticsLogic::ReadCarrierDetect(AttributeValueEncoder & encoder) const
{
    Attributes::CarrierDetect::TypeInfo::Type carrierDetect;
    bool value = false;

    if (GetDiagnosticDataProvider().GetEthCarrierDetect(value) == CHIP_NO_ERROR)
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

BitFlags<EthernetNetworkDiagnostics::Feature> EthernetDiagnosticsLogic::GetFeatureMap() const
{
    return BitFlags<EthernetNetworkDiagnostics::Feature>()
        .Set(EthernetNetworkDiagnostics::Feature::kPacketCounts, mEnabledAttributes.enablePacketCount)
        .Set(EthernetNetworkDiagnostics::Feature::kErrorCounts, mEnabledAttributes.enableErrCount);
}

CHIP_ERROR EthernetDiagnosticsLogic::AcceptedCommands(ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    BitFlags<EthernetNetworkDiagnostics::Feature> featureMap = GetFeatureMap();

    if (featureMap.Has(EthernetNetworkDiagnostics::Feature::kPacketCounts) ||
        featureMap.Has(EthernetNetworkDiagnostics::Feature::kErrorCounts))
    {
        static constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = { Commands::ResetCounts::kMetadataEntry };
        return builder.ReferenceExisting(kAcceptedCommands);
    }

    // no commands supported
    return CHIP_NO_ERROR;
}

CHIP_ERROR EthernetDiagnosticsLogic::Attributes(ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    struct Mapping
    {
        const bool enable;
        const DataModel::AttributeEntry & entry;
    };

    const Mapping mappings[] = {
        { mEnabledAttributes.enableCarrierDetect, Attributes::CarrierDetect::kMetadataEntry },
        { mEnabledAttributes.enableErrCount, Attributes::CollisionCount::kMetadataEntry },
        { mEnabledAttributes.enableFullDuplex, Attributes::FullDuplex::kMetadataEntry },
        { mEnabledAttributes.enableErrCount, Attributes::OverrunCount::kMetadataEntry },
        { mEnabledAttributes.enablePacketCount, Attributes::PacketRxCount::kMetadataEntry },
        { mEnabledAttributes.enablePacketCount, Attributes::PacketTxCount::kMetadataEntry },
        { mEnabledAttributes.enablePHYRate, Attributes::PHYRate::kMetadataEntry },
        { mEnabledAttributes.enableTimeSinceReset, Attributes::TimeSinceReset::kMetadataEntry },
        { mEnabledAttributes.enableErrCount, Attributes::TxErrCount::kMetadataEntry },
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
