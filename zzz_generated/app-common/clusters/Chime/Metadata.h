// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Chime (cluster code: 1366/0x556)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/Chime/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Chime {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace InstalledChimeSounds {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::InstalledChimeSounds::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace InstalledChimeSounds
namespace SelectedChime {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::SelectedChime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace SelectedChime
namespace Enabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::Enabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace Enabled

} // namespace Attributes

namespace Commands {
namespace PlayChimeSound {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::PlayChimeSound::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace PlayChimeSound

} // namespace Commands
} // namespace Chime
} // namespace Clusters
} // namespace app
} // namespace chip
