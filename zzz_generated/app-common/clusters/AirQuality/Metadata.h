// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AirQuality (cluster code: 91/0x5B)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/AirQuality/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AirQuality {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace AirQuality {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AirQuality::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace AirQuality

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace AirQuality
} // namespace Clusters
} // namespace app
} // namespace chip
