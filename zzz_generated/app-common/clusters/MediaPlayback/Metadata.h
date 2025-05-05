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
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CurrentState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentState
namespace StartTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = StartTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace StartTime
namespace Duration {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Duration::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Duration
namespace SampledPosition {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SampledPosition::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SampledPosition
namespace PlaybackSpeed {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PlaybackSpeed::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PlaybackSpeed
namespace SeekRangeEnd {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SeekRangeEnd::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SeekRangeEnd
namespace SeekRangeStart {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SeekRangeStart::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SeekRangeStart
namespace ActiveAudioTrack {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ActiveAudioTrack::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ActiveAudioTrack
namespace AvailableAudioTracks {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AvailableAudioTracks::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace AvailableAudioTracks
namespace ActiveTextTrack {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ActiveTextTrack::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ActiveTextTrack
namespace AvailableTextTracks {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AvailableTextTracks::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace AvailableTextTracks

} // namespace Attributes

namespace Commands {
namespace Play {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Play::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace Play
namespace Pause {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Pause::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace Pause
namespace Stop {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Stop::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace Stop
namespace StartOver {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = StartOver::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace StartOver
namespace Previous {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Previous::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace Previous
namespace Next {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Next::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace Next
namespace Rewind {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Rewind::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace Rewind
namespace FastForward {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = FastForward::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace FastForward
namespace SkipForward {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = SkipForward::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace SkipForward
namespace SkipBackward {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = SkipBackward::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace SkipBackward
namespace Seek {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Seek::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace Seek
namespace ActivateAudioTrack {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ActivateAudioTrack::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace ActivateAudioTrack
namespace ActivateTextTrack {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ActivateTextTrack::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace ActivateTextTrack
namespace DeactivateTextTrack {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = DeactivateTextTrack::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace DeactivateTextTrack

} // namespace Commands
} // namespace MediaPlayback
} // namespace Clusters
} // namespace app
} // namespace chip
