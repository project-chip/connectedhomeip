// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster GeneralCommissioning (cluster code: 48/0x30)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/GeneralCommissioning/Ids.h>
#include <clusters/GeneralCommissioning/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::GeneralCommissioning::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::GeneralCommissioning::Attributes;
        switch (commandId)
        {
        case Breadcrumb::Id:
            return Breadcrumb::kMetadataEntry;
        case BasicCommissioningInfo::Id:
            return BasicCommissioningInfo::kMetadataEntry;
        case RegulatoryConfig::Id:
            return RegulatoryConfig::kMetadataEntry;
        case LocationCapability::Id:
            return LocationCapability::kMetadataEntry;
        case SupportsConcurrentConnection::Id:
            return SupportsConcurrentConnection::kMetadataEntry;
        case TCAcceptedVersion::Id:
            return TCAcceptedVersion::kMetadataEntry;
        case TCMinRequiredVersion::Id:
            return TCMinRequiredVersion::kMetadataEntry;
        case TCAcknowledgements::Id:
            return TCAcknowledgements::kMetadataEntry;
        case TCAcknowledgementsRequired::Id:
            return TCAcknowledgementsRequired::kMetadataEntry;
        case TCUpdateDeadline::Id:
            return TCUpdateDeadline::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::GeneralCommissioning::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::GeneralCommissioning::Commands;
        switch (commandId)
        {
        case ArmFailSafe::Id:
            return ArmFailSafe::kMetadataEntry;
        case SetRegulatoryConfig::Id:
            return SetRegulatoryConfig::kMetadataEntry;
        case CommissioningComplete::Id:
            return CommissioningComplete::kMetadataEntry;
        case SetTCAcknowledgements::Id:
            return SetTCAcknowledgements::kMetadataEntry;

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
