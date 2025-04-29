// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster SoilMeasurement (cluster code: 1072/0x430)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/SoilMeasurement/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace SoilMeasurement {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace SoilMoistureMeasurementLimits {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SoilMoistureMeasurementLimits::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SoilMoistureMeasurementLimits
namespace SoilMoistureMeasuredValue {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SoilMoistureMeasuredValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SoilMoistureMeasuredValue

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace SoilMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
