// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Groupcast (cluster code: 101/0x65)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/Groupcast/Ids.h>
#include <clusters/Groupcast/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::Groupcast::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::Groupcast::Attributes;
        switch (attributeId)
        {
        case Membership::Id:
            return Membership::kMetadataEntry;
        case MaxMembershipCount::Id:
            return MaxMembershipCount::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::Groupcast::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::Groupcast::Commands;
        switch (commandId)
        {
        case JoinGroup::Id:
            return JoinGroup::kMetadataEntry;
        case LeaveGroup::Id:
            return LeaveGroup::kMetadataEntry;
        case UpdateGroupKey::Id:
            return UpdateGroupKey::kMetadataEntry;
        case ExpireGracePeriod::Id:
            return ExpireGracePeriod::kMetadataEntry;
        case ConfigureAuxiliaryACL::Id:
            return ConfigureAuxiliaryACL::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
