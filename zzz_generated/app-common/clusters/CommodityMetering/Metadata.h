// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster CommodityMetering (cluster code: 2823/0xB07)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/CommodityMetering/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommodityMetering {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace MeteredQuantity {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(MeteredQuantity::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace MeteredQuantity
namespace MeteredQuantityTimestamp {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MeteredQuantityTimestamp::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace MeteredQuantityTimestamp
namespace TariffUnit {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TariffUnit::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TariffUnit

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace CommodityMetering
} // namespace Clusters
} // namespace app
} // namespace chip
