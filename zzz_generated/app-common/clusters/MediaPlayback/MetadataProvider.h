// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster MediaPlayback (cluster code: 1286/0x506)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/MediaPlayback/Ids.h>
#include <clusters/MediaPlayback/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::MediaPlayback::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::MediaPlayback::Attributes;
        switch (attributeId)
        {
        case CurrentState::Id:
            return CurrentState::kMetadataEntry;
        case StartTime::Id:
            return StartTime::kMetadataEntry;
        case Duration::Id:
            return Duration::kMetadataEntry;
        case SampledPosition::Id:
            return SampledPosition::kMetadataEntry;
        case PlaybackSpeed::Id:
            return PlaybackSpeed::kMetadataEntry;
        case SeekRangeEnd::Id:
            return SeekRangeEnd::kMetadataEntry;
        case SeekRangeStart::Id:
            return SeekRangeStart::kMetadataEntry;
        case ActiveAudioTrack::Id:
            return ActiveAudioTrack::kMetadataEntry;
        case AvailableAudioTracks::Id:
            return AvailableAudioTracks::kMetadataEntry;
        case ActiveTextTrack::Id:
            return ActiveTextTrack::kMetadataEntry;
        case AvailableTextTracks::Id:
            return AvailableTextTracks::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::MediaPlayback::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::MediaPlayback::Commands;
        switch (commandId)
        {
        case Play::Id:
            return Play::kMetadataEntry;
        case Pause::Id:
            return Pause::kMetadataEntry;
        case Stop::Id:
            return Stop::kMetadataEntry;
        case StartOver::Id:
            return StartOver::kMetadataEntry;
        case Previous::Id:
            return Previous::kMetadataEntry;
        case Next::Id:
            return Next::kMetadataEntry;
        case Rewind::Id:
            return Rewind::kMetadataEntry;
        case FastForward::Id:
            return FastForward::kMetadataEntry;
        case SkipForward::Id:
            return SkipForward::kMetadataEntry;
        case SkipBackward::Id:
            return SkipBackward::kMetadataEntry;
        case Seek::Id:
            return Seek::kMetadataEntry;
        case ActivateAudioTrack::Id:
            return ActivateAudioTrack::kMetadataEntry;
        case ActivateTextTrack::Id:
            return ActivateTextTrack::kMetadataEntry;
        case DeactivateTextTrack::Id:
            return DeactivateTextTrack::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
