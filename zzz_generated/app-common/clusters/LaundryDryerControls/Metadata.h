// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster LaundryDryerControls (cluster code: 74/0x4A)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/LaundryDryerControls/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace LaundryDryerControls {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace SupportedDrynessLevels {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::SupportedDrynessLevels::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SupportedDrynessLevels
namespace SelectedDrynessLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::SelectedDrynessLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace SelectedDrynessLevel

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace LaundryDryerControls
} // namespace Clusters
} // namespace app
} // namespace chip
