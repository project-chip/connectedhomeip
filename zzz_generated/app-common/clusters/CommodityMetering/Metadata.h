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
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::MeteredQuantity::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MeteredQuantity
namespace MeteredQuantityTimestamp {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::MeteredQuantityTimestamp::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MeteredQuantityTimestamp
namespace MeasurementType {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::MeasurementType::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MeasurementType

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace CommodityMetering
} // namespace Clusters
} // namespace app
} // namespace chip
