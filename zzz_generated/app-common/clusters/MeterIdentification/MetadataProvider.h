// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster MeterIdentification (cluster code: 2822/0xB06)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/MeterIdentification/Ids.h>
#include <clusters/MeterIdentification/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::MeterIdentification::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::MeterIdentification::Attributes;
        switch (attributeId)
        {
        case MeterType::Id:
            return MeterType::kMetadataEntry;
        case PointOfDelivery::Id:
            return PointOfDelivery::kMetadataEntry;
        case MeterSerialNumber::Id:
            return MeterSerialNumber::kMetadataEntry;
        case ProtocolVersion::Id:
            return ProtocolVersion::kMetadataEntry;
        case PowerThreshold::Id:
            return PowerThreshold::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::MeterIdentification::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::MeterIdentification::Commands;
        switch (commandId)
        {

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
