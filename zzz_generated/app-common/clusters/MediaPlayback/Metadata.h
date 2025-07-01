// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster MediaPlayback (cluster code: 1286/0x506)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/MediaPlayback/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MediaPlayback {

inline constexpr uint32_t kRevision = 2;

namespace Attributes {
namespace CurrentState {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentState::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentState
namespace StartTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry(StartTime::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace StartTime
namespace Duration {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Duration::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Duration
namespace SampledPosition {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SampledPosition::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SampledPosition
namespace PlaybackSpeed {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PlaybackSpeed::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PlaybackSpeed
namespace SeekRangeEnd {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SeekRangeEnd::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SeekRangeEnd
namespace SeekRangeStart {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SeekRangeStart::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SeekRangeStart
namespace ActiveAudioTrack {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ActiveAudioTrack::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ActiveAudioTrack
namespace AvailableAudioTracks {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(AvailableAudioTracks::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace AvailableAudioTracks
namespace ActiveTextTrack {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ActiveTextTrack::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ActiveTextTrack
namespace AvailableTextTracks {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(AvailableTextTracks::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace AvailableTextTracks

} // namespace Attributes

namespace Commands {
namespace Play {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Play::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace Play
namespace Pause {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Pause::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace Pause
namespace Stop {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Stop::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace Stop
namespace StartOver {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(StartOver::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace StartOver
namespace Previous {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Previous::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace Previous
namespace Next {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Next::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace Next
namespace Rewind {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Rewind::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace Rewind
namespace FastForward {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(FastForward::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace FastForward
namespace SkipForward {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(SkipForward::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace SkipForward
namespace SkipBackward {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(SkipBackward::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace SkipBackward
namespace Seek {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Seek::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace Seek
namespace ActivateAudioTrack {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ActivateAudioTrack::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace ActivateAudioTrack
namespace ActivateTextTrack {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ActivateTextTrack::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace ActivateTextTrack
namespace DeactivateTextTrack {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(DeactivateTextTrack::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace DeactivateTextTrack

} // namespace Commands
} // namespace MediaPlayback
} // namespace Clusters
} // namespace app
} // namespace chip
