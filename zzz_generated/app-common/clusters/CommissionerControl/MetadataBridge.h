// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster CommissionerControl (cluster code: 1873/0x751)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/CommissionerControl/Ids.h>
#include <clusters/CommissionerControl/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::CommissionerControl::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::CommissionerControl::Attributes;
        switch (commandId)
        {
        case SupportedDeviceCategories::Id:
            return SupportedDeviceCategories::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::CommissionerControl::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::CommissionerControl::Commands;
        switch (commandId)
        {
        case RequestCommissioningApproval::Id:
            return RequestCommissioningApproval::kMetadataEntry;
        case CommissionNode::Id:
            return CommissionNode::kMetadataEntry;

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
