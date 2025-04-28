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
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MPTZPosition::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MPTZPosition
namespace MaxPresets {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MaxPresets::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxPresets
namespace MPTZPresets {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MPTZPresets::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MPTZPresets
namespace DPTZRelativeMove {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = DPTZRelativeMove::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace DPTZRelativeMove
namespace ZoomMax {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ZoomMax::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ZoomMax
namespace TiltMin {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TiltMin::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TiltMin
namespace TiltMax {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TiltMax::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TiltMax
namespace PanMin {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PanMin::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PanMin
namespace PanMax {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PanMax::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PanMax

} // namespace Attributes

namespace Commands {
namespace MPTZSetPosition {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = MPTZSetPosition::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace MPTZSetPosition
namespace MPTZRelativeMove {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = MPTZRelativeMove::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace MPTZRelativeMove
namespace MPTZMoveToPreset {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = MPTZMoveToPreset::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace MPTZMoveToPreset
namespace MPTZSavePreset {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = MPTZSavePreset::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace MPTZSavePreset
namespace MPTZRemovePreset {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = MPTZRemovePreset::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace MPTZRemovePreset
namespace DPTZSetViewport {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = DPTZSetViewport::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace DPTZSetViewport
namespace DPTZRelativeMove {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = DPTZRelativeMove::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace DPTZRelativeMove

} // namespace Commands
} // namespace CameraAvSettingsUserLevelManagement
} // namespace Clusters
} // namespace app
} // namespace chip
