// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster MicrowaveOvenMode (cluster code: 94/0x5E)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/MicrowaveOvenMode/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MicrowaveOvenMode {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace SupportedModes {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(SupportedModes::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace SupportedModes
namespace CurrentMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentMode::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentMode

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace MicrowaveOvenMode
} // namespace Clusters
} // namespace app
} // namespace chip
