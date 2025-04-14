// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster DemandResponseLoadControl (cluster code: 150/0x96)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/DemandResponseLoadControl/Ids.h>
#include <clusters/DemandResponseLoadControl/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::DemandResponseLoadControl::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::DemandResponseLoadControl::Attributes;
        switch (commandId)
        {
        case LoadControlPrograms::Id:
            return LoadControlPrograms::kMetadataEntry;
        case NumberOfLoadControlPrograms::Id:
            return NumberOfLoadControlPrograms::kMetadataEntry;
        case Events::Id:
            return Events::kMetadataEntry;
        case ActiveEvents::Id:
            return ActiveEvents::kMetadataEntry;
        case NumberOfEventsPerProgram::Id:
            return NumberOfEventsPerProgram::kMetadataEntry;
        case NumberOfTransitions::Id:
            return NumberOfTransitions::kMetadataEntry;
        case DefaultRandomStart::Id:
            return DefaultRandomStart::kMetadataEntry;
        case DefaultRandomDuration::Id:
            return DefaultRandomDuration::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::DemandResponseLoadControl::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::DemandResponseLoadControl::Commands;
        switch (commandId)
        {
        case RegisterLoadControlProgramRequest::Id:
            return RegisterLoadControlProgramRequest::kMetadataEntry;
        case UnregisterLoadControlProgramRequest::Id:
            return UnregisterLoadControlProgramRequest::kMetadataEntry;
        case AddLoadControlEventRequest::Id:
            return AddLoadControlEventRequest::kMetadataEntry;
        case RemoveLoadControlEventRequest::Id:
            return RemoveLoadControlEventRequest::kMetadataEntry;
        case ClearLoadControlEventsRequest::Id:
            return ClearLoadControlEventsRequest::kMetadataEntry;

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
