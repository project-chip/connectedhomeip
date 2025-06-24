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
    BitFlags<EthernetNetworkDiagnostics::Feature> featureMap;

    // Check PacketCounts feature - enabled if either Rx or Tx packet counts are supported
    if ((mEnabledAttributes.enablePacketRxCount || mEnabledAttributes.enablePacketTxCount))
    {
        uint64_t tempValue;
        bool supportsPacketCounts = false;

        if (mEnabledAttributes.enablePacketRxCount &&
            GetDiagnosticDataProvider().GetEthPacketRxCount(tempValue) != CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
        {
            supportsPacketCounts = true;
        }
        else if (mEnabledAttributes.enablePacketTxCount &&
                 GetDiagnosticDataProvider().GetEthPacketTxCount(tempValue) != CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
        {
            supportsPacketCounts = true;
        }

        featureMap.Set(EthernetNetworkDiagnostics::Feature::kPacketCounts, supportsPacketCounts);
    }

    // Check ErrorCounts feature - enabled if any error count attributes are supported
    if ((mEnabledAttributes.enableTxErrCount || mEnabledAttributes.enableCollisionCount || mEnabledAttributes.enableOverrunCount))
    {
        uint64_t tempValue;
        bool supportsErrorCounts = false;

        if (mEnabledAttributes.enableTxErrCount &&
            GetDiagnosticDataProvider().GetEthTxErrCount(tempValue) != CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
        {
            supportsErrorCounts = true;
        }
        else if (mEnabledAttributes.enableCollisionCount &&
                 GetDiagnosticDataProvider().GetEthCollisionCount(tempValue) != CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
        {
            supportsErrorCounts = true;
        }
        else if (mEnabledAttributes.enableOverrunCount &&
                 GetDiagnosticDataProvider().GetEthOverrunCount(tempValue) != CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
        {
            supportsErrorCounts = true;
        }

        featureMap.Set(EthernetNetworkDiagnostics::Feature::kErrorCounts, supportsErrorCounts);
    }

    return featureMap;
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
    // ensure we have space for all attributes. We may add at most 9 attributes (all optional)
    ReturnErrorOnFailure(builder.EnsureAppendCapacity(9 + DefaultServerCluster::GlobalAttributes().size()));

    if (mEnabledAttributes.enableCarrierDetect)
    {
        ReturnErrorOnFailure(builder.Append(Attributes::CarrierDetect::kMetadataEntry));
    }

    if (mEnabledAttributes.enableCollisionCount)
    {
        ReturnErrorOnFailure(builder.Append(Attributes::CollisionCount::kMetadataEntry));
    }

    if (mEnabledAttributes.enableFullDuplex)
    {
        ReturnErrorOnFailure(builder.Append(Attributes::FullDuplex::kMetadataEntry));
    }

    if (mEnabledAttributes.enableOverrunCount)
    {
        ReturnErrorOnFailure(builder.Append(Attributes::OverrunCount::kMetadataEntry));
    }

    if (mEnabledAttributes.enablePacketRxCount)
    {
        ReturnErrorOnFailure(builder.Append(Attributes::PacketRxCount::kMetadataEntry));
    }

    if (mEnabledAttributes.enablePacketTxCount)
    {
        ReturnErrorOnFailure(builder.Append(Attributes::PacketTxCount::kMetadataEntry));
    }

    if (mEnabledAttributes.enablePHYRate)
    {
        ReturnErrorOnFailure(builder.Append(Attributes::PHYRate::kMetadataEntry));
    }

    if (mEnabledAttributes.enableTimeSinceReset)
    {
        ReturnErrorOnFailure(builder.Append(Attributes::TimeSinceReset::kMetadataEntry));
    }

    if (mEnabledAttributes.enableTxErrCount)
    {
        ReturnErrorOnFailure(builder.Append(Attributes::TxErrCount::kMetadataEntry));
    }

    return builder.AppendElements(DefaultServerCluster::GlobalAttributes());
}

} // namespace Clusters
} // namespace app
} // namespace chip
