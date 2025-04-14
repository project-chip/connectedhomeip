// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Timer (cluster code: 71/0x47)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/Timer/Ids.h>
#include <clusters/Timer/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::Timer::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::Timer::Attributes;
        switch (commandId)
        {
        case SetTime::Id:
            return SetTime::kMetadataEntry;
        case TimeRemaining::Id:
            return TimeRemaining::kMetadataEntry;
        case TimerState::Id:
            return TimerState::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::Timer::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::Timer::Commands;
        switch (commandId)
        {
        case SetTimer::Id:
            return SetTimer::kMetadataEntry;
        case ResetTimer::Id:
            return ResetTimer::kMetadataEntry;
        case AddTime::Id:
            return AddTime::kMetadataEntry;
        case ReduceTime::Id:
            return ReduceTime::kMetadataEntry;

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
