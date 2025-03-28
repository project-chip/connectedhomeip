// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster LaundryDryerControls (cluster code: 74/0x4A)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/LaundryDryerControls/LaundryDryerControlsIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace LaundryDryerControls {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kSupportedDrynessLevelsEntry = {
    .attributeId    = LaundryDryerControls::Attributes::SupportedDrynessLevels::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSelectedDrynessLevelEntry = {
    .attributeId    = LaundryDryerControls::Attributes::SelectedDrynessLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace Metadata
} // namespace LaundryDryerControls
} // namespace clusters
} // namespace app
} // namespace chip
