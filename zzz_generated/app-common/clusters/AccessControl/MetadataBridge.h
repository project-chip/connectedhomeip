// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AccessControl (cluster code: 31/0x1F)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/AccessControl/Ids.h>
#include <clusters/AccessControl/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::AccessControl::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::AccessControl::Attributes;
        switch (commandId)
        {
        case Acl::Id:
            return Acl::kMetadataEntry;
        case Extension::Id:
            return Extension::kMetadataEntry;
        case SubjectsPerAccessControlEntry::Id:
            return SubjectsPerAccessControlEntry::kMetadataEntry;
        case TargetsPerAccessControlEntry::Id:
            return TargetsPerAccessControlEntry::kMetadataEntry;
        case AccessControlEntriesPerFabric::Id:
            return AccessControlEntriesPerFabric::kMetadataEntry;
        case CommissioningARL::Id:
            return CommissioningARL::kMetadataEntry;
        case Arl::Id:
            return Arl::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::AccessControl::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::AccessControl::Commands;
        switch (commandId)
        {
        case ReviewFabricRestrictions::Id:
            return ReviewFabricRestrictions::kMetadataEntry;

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
