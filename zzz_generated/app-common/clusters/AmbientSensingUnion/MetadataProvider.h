// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AmbientSensingUnion (cluster code: 1074/0x432)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/AmbientSensingUnion/Ids.h>
#include <clusters/AmbientSensingUnion/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::AmbientSensingUnion::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::AmbientSensingUnion::Attributes;
        switch (attributeId)
        {
        case UnionName::Id:
            return UnionName::kMetadataEntry;
        case UnionHealth::Id:
            return UnionHealth::kMetadataEntry;
        case UnionContributorList::Id:
            return UnionContributorList::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::AmbientSensingUnion::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::AmbientSensingUnion::Commands;
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
