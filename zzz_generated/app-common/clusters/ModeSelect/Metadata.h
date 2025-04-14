// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ModeSelect (cluster code: 80/0x50)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ModeSelect/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ModeSelect {

inline constexpr uint32_t kRevision = 2;

namespace Attributes {
namespace Description {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::Description::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Description
namespace StandardNamespace {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::StandardNamespace::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace StandardNamespace
namespace SupportedModes {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::SupportedModes::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SupportedModes
namespace CurrentMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::CurrentMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentMode
namespace StartUpMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::StartUpMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace StartUpMode
namespace OnMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::OnMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace OnMode

} // namespace Attributes

namespace Commands {
namespace ChangeToMode {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::ChangeToMode::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace ChangeToMode

} // namespace Commands
} // namespace ModeSelect
} // namespace Clusters
} // namespace app
} // namespace chip
