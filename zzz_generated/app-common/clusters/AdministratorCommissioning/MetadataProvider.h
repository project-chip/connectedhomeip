// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AdministratorCommissioning (cluster code: 60/0x3C)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/AdministratorCommissioning/Ids.h>
#include <clusters/AdministratorCommissioning/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::AdministratorCommissioning::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::AdministratorCommissioning::Attributes;
        switch (attributeId)
        {
        case WindowStatus::Id:
            return WindowStatus::kMetadataEntry;
        case AdminFabricIndex::Id:
            return AdminFabricIndex::kMetadataEntry;
        case AdminVendorId::Id:
            return AdminVendorId::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::AdministratorCommissioning::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::AdministratorCommissioning::Commands;
        switch (commandId)
        {
        case OpenCommissioningWindow::Id:
            return OpenCommissioningWindow::kMetadataEntry;
        case OpenBasicCommissioningWindow::Id:
            return OpenBasicCommissioningWindow::kMetadataEntry;
        case RevokeCommissioning::Id:
            return RevokeCommissioning::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
