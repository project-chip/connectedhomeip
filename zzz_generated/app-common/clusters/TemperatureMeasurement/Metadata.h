// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster TemperatureMeasurement (cluster code: 1026/0x402)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/TemperatureMeasurement/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace TemperatureMeasurement {

inline constexpr uint32_t kRevision = 4;

namespace Attributes {
namespace MeasuredValue {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MeasuredValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MeasuredValue
namespace MinMeasuredValue {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MinMeasuredValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MinMeasuredValue
namespace MaxMeasuredValue {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MaxMeasuredValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxMeasuredValue
namespace Tolerance {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Tolerance::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Tolerance

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace TemperatureMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
