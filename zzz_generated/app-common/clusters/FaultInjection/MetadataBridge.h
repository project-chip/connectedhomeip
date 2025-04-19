// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster FaultInjection (cluster code: 4294048774/0xFFF1FC06)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/FaultInjection/Ids.h>
#include <clusters/FaultInjection/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::FaultInjection::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::FaultInjection::Attributes;
        switch (commandId)
        {

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::FaultInjection::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::FaultInjection::Commands;
        switch (commandId)
        {
        case FailAtFault::Id:
            return FailAtFault::kMetadataEntry;
        case FailRandomlyAtFault::Id:
            return FailRandomlyAtFault::kMetadataEntry;

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
