// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster CameraAvSettingsUserLevelManagement (cluster code: 1362/0x552)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/CameraAvSettingsUserLevelManagement/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CameraAvSettingsUserLevelManagement {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace MPTZPosition {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MPTZPosition::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MPTZPosition
namespace MaxPresets {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxPresets::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxPresets
namespace MPTZPresets {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(MPTZPresets::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace MPTZPresets
namespace DPTZStreams {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(DPTZStreams::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace DPTZStreams
namespace ZoomMax {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ZoomMax::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ZoomMax
namespace TiltMin {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TiltMin::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TiltMin
namespace TiltMax {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TiltMax::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TiltMax
namespace PanMin {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PanMin::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PanMin
namespace PanMax {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PanMax::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PanMax

} // namespace Attributes

namespace Commands {
namespace MPTZSetPosition {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(MPTZSetPosition::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace MPTZSetPosition
namespace MPTZRelativeMove {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(MPTZRelativeMove::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace MPTZRelativeMove
namespace MPTZMoveToPreset {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(MPTZMoveToPreset::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace MPTZMoveToPreset
namespace MPTZSavePreset {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(MPTZSavePreset::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace MPTZSavePreset
namespace MPTZRemovePreset {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(MPTZRemovePreset::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace MPTZRemovePreset
namespace DPTZSetViewport {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(DPTZSetViewport::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace DPTZSetViewport
namespace DPTZRelativeMove {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(DPTZRelativeMove::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace DPTZRelativeMove

} // namespace Commands
} // namespace CameraAvSettingsUserLevelManagement
} // namespace Clusters
} // namespace app
} // namespace chip
