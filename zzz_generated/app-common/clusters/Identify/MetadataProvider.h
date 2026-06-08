// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Identify (cluster code: 3/0x3)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/Identify/Ids.h>
#include <clusters/Identify/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::Identify::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::Identify::Attributes;
        switch (attributeId)
        {
        case IdentifyTime::Id:
            return IdentifyTime::kMetadataEntry;
        case IdentifyType::Id:
            return IdentifyType::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::Identify::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::Identify::Commands;
        switch (commandId)
        {
        case Identify::Id:
            return Identify::kMetadataEntry;
        case TriggerEffect::Id:
            return TriggerEffect::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
