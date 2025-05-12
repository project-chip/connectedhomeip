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
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(SupportedDrynessLevels::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace SupportedDrynessLevels
namespace SelectedDrynessLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SelectedDrynessLevel::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace SelectedDrynessLevel

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace LaundryDryerControls
} // namespace Clusters
} // namespace app
} // namespace chip
