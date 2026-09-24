// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AudioControl (cluster code: 1298/0x512)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/AudioControl/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AudioControl {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

namespace SoftMuted {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SoftMuted::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SoftMuted
namespace PhysicallyMuted {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PhysicallyMuted::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PhysicallyMuted
namespace Volume {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Volume::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Volume
namespace MinDeviceVolume {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MinDeviceVolume::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MinDeviceVolume
namespace MaxDeviceVolume {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxDeviceVolume::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxDeviceVolume
namespace MaxDeviceVolumeDB {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxDeviceVolumeDB::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxDeviceVolumeDB
namespace MaxUserVolume {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxUserVolume::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace MaxUserVolume
namespace DefaultStepSize {
inline constexpr DataModel::AttributeEntry kMetadataEntry(DefaultStepSize::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace DefaultStepSize
namespace SetVolumeUnmutePolicy {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SetVolumeUnmutePolicy::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace SetVolumeUnmutePolicy
namespace IncreaseVolumeUnmutePolicy {
inline constexpr DataModel::AttributeEntry kMetadataEntry(IncreaseVolumeUnmutePolicy::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kManage);
} // namespace IncreaseVolumeUnmutePolicy
namespace IncreaseVolumeUnmuteVolume {
inline constexpr DataModel::AttributeEntry kMetadataEntry(IncreaseVolumeUnmuteVolume::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kManage);
} // namespace IncreaseVolumeUnmuteVolume
namespace DecreaseVolumeUnmutePolicy {
inline constexpr DataModel::AttributeEntry kMetadataEntry(DecreaseVolumeUnmutePolicy::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kManage);
} // namespace DecreaseVolumeUnmutePolicy
namespace StartUpMuted {
inline constexpr DataModel::AttributeEntry kMetadataEntry(StartUpMuted::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace StartUpMuted
namespace StartUpVolume {
inline constexpr DataModel::AttributeEntry kMetadataEntry(StartUpVolume::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace StartUpVolume
namespace Bass {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Bass::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Bass
namespace Mid {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Mid::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Mid
namespace Treble {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Treble::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Treble
namespace MinCorrection {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MinCorrection::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MinCorrection
namespace MaxCorrection {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxCorrection::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxCorrection
constexpr std::array<DataModel::AttributeEntry, 9> kMandatoryMetadata = {
    SoftMuted::kMetadataEntry,
    Volume::kMetadataEntry,
    MinDeviceVolume::kMetadataEntry,
    MaxDeviceVolume::kMetadataEntry,
    DefaultStepSize::kMetadataEntry,
    SetVolumeUnmutePolicy::kMetadataEntry,
    IncreaseVolumeUnmutePolicy::kMetadataEntry,
    IncreaseVolumeUnmuteVolume::kMetadataEntry,
    DecreaseVolumeUnmutePolicy::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {

namespace Mute {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Mute::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace Mute
namespace Unmute {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Unmute::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace Unmute
namespace ToggleMuted {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ToggleMuted::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace ToggleMuted
namespace SetVolume {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(SetVolume::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace SetVolume
namespace IncreaseVolume {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(IncreaseVolume::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace IncreaseVolume
namespace DecreaseVolume {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(DecreaseVolume::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace DecreaseVolume

} // namespace Commands

namespace Events {} // namespace Events
} // namespace AudioControl
} // namespace Clusters
} // namespace app
} // namespace chip
