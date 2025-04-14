// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster IlluminanceMeasurement (cluster code: 1024/0x400)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/IlluminanceMeasurement/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace IlluminanceMeasurement {

inline constexpr uint32_t kRevision = 3;

namespace Attributes {
namespace MeasuredValue {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::MeasuredValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MeasuredValue
namespace MinMeasuredValue {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::MinMeasuredValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MinMeasuredValue
namespace MaxMeasuredValue {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::MaxMeasuredValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxMeasuredValue
namespace Tolerance {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::Tolerance::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Tolerance
namespace LightSensorType {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::LightSensorType::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace LightSensorType

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace IlluminanceMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
