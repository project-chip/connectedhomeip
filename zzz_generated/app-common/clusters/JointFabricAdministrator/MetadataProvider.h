// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster JointFabricAdministrator (cluster code: 1875/0x753)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/JointFabricAdministrator/Ids.h>
#include <clusters/JointFabricAdministrator/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::JointFabricAdministrator::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::JointFabricAdministrator::Attributes;
        switch (attributeId)
        {
        case AdministratorFabricIndex::Id:
            return AdministratorFabricIndex::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::JointFabricAdministrator::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::JointFabricAdministrator::Commands;
        switch (commandId)
        {
        case ICACCSRRequest::Id:
            return ICACCSRRequest::kMetadataEntry;
        case AddICAC::Id:
            return AddICAC::kMetadataEntry;
        case OpenJointCommissioningWindow::Id:
            return OpenJointCommissioningWindow::kMetadataEntry;
        case TransferAnchorRequest::Id:
            return TransferAnchorRequest::kMetadataEntry;
        case TransferAnchorComplete::Id:
            return TransferAnchorComplete::kMetadataEntry;
        case AnnounceJointFabricAdministrator::Id:
            return AnnounceJointFabricAdministrator::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
