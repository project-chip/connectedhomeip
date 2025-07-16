// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Channel (cluster code: 1284/0x504)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/Channel/Ids.h>
#include <clusters/Channel/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::Channel::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::Channel::Attributes;
        switch (attributeId)
        {
        case ChannelList::Id:
            return ChannelList::kMetadataEntry;
        case Lineup::Id:
            return Lineup::kMetadataEntry;
        case CurrentChannel::Id:
            return CurrentChannel::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::Channel::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::Channel::Commands;
        switch (commandId)
        {
        case ChangeChannel::Id:
            return ChangeChannel::kMetadataEntry;
        case ChangeChannelByNumber::Id:
            return ChangeChannelByNumber::kMetadataEntry;
        case SkipChannel::Id:
            return SkipChannel::kMetadataEntry;
        case GetProgramGuide::Id:
            return GetProgramGuide::kMetadataEntry;
        case RecordProgram::Id:
            return RecordProgram::kMetadataEntry;
        case CancelRecordProgram::Id:
            return CancelRecordProgram::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
