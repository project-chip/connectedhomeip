// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster PowerTopology (cluster code: 156/0x9C)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/PowerTopology/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace PowerTopology {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace AvailableEndpoints {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(AvailableEndpoints::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace AvailableEndpoints
namespace ActiveEndpoints {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ActiveEndpoints::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace ActiveEndpoints

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace PowerTopology
} // namespace Clusters
} // namespace app
} // namespace chip
