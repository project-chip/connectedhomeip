// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster PowerSourceConfiguration (cluster code: 46/0x2E)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/PowerSourceConfiguration/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace PowerSourceConfiguration {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
inline constexpr uint32_t kMetadataCount = 6;

namespace Sources {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(Sources::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace Sources

} // namespace Attributes

namespace Commands {
inline constexpr uint32_t kMetadataCount = 0;

} // namespace Commands
} // namespace PowerSourceConfiguration
} // namespace Clusters
} // namespace app
} // namespace chip
