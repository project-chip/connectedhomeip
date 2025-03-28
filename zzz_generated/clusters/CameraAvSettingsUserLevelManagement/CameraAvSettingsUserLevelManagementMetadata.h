// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster CameraAvSettingsUserLevelManagement (cluster code: 1362/0x552)
// based on src/controller/data_model/controller-clusters.matter

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/CameraAvSettingsUserLevelManagement/CameraAvSettingsUserLevelManagementIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace CameraAvSettingsUserLevelManagement {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kMPTZPositionEntry = {
    .attributeId    = Attributes::MPTZPosition::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMaxPresetsEntry = {
    .attributeId    = Attributes::MaxPresets::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMPTZPresetsEntry = {
    .attributeId    = Attributes::MPTZPresets::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kDPTZRelativeMoveEntry = {
    .attributeId    = Attributes::DPTZRelativeMove::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kZoomMaxEntry = {
    .attributeId    = Attributes::ZoomMax::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTiltMinEntry = {
    .attributeId    = Attributes::TiltMin::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTiltMaxEntry = {
    .attributeId    = Attributes::TiltMax::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPanMinEntry = {
    .attributeId    = Attributes::PanMin::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPanMaxEntry = {
    .attributeId    = Attributes::PanMax::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kMPTZSetPositionEntry = {
    .commandId       = Commands::MPTZSetPosition::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kMPTZRelativeMoveEntry = {
    .commandId       = Commands::MPTZRelativeMove::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kMPTZMoveToPresetEntry = {
    .commandId       = Commands::MPTZMoveToPreset::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kMPTZSavePresetEntry = {
    .commandId       = Commands::MPTZSavePreset::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kMPTZRemovePresetEntry = {
    .commandId       = Commands::MPTZRemovePreset::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kDPTZSetViewportEntry = {
    .commandId       = Commands::DPTZSetViewport::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kDPTZRelativeMoveEntry = {
    .commandId       = Commands::DPTZRelativeMove::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace CameraAvSettingsUserLevelManagement
} // namespace clusters
} // namespace app
} // namespace chip
