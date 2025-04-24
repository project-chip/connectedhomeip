// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster TargetNavigator (cluster code: 1285/0x505)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/TargetNavigator/Ids.h>
#include <clusters/TargetNavigator/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::TargetNavigator::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::TargetNavigator::Attributes;
        switch (commandId)
        {
        case TargetList::Id:
            return TargetList::kMetadataEntry;
        case CurrentTarget::Id:
            return CurrentTarget::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::TargetNavigator::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::TargetNavigator::Commands;
        switch (commandId)
        {
        case NavigateTarget::Id:
            return NavigateTarget::kMetadataEntry;

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
