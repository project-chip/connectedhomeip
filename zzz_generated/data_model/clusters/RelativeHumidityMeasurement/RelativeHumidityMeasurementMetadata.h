// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster RelativeHumidityMeasurement (cluster code: 1029/0x405)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/RelativeHumidityMeasurement/RelativeHumidityMeasurementIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace RelativeHumidityMeasurement {
namespace Metadata {

inline constexpr uint32_t kRevision = 3;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kMeasuredValueEntry = {
    .attributeId    = RelativeHumidityMeasurement::Attributes::MeasuredValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMinMeasuredValueEntry = {
    .attributeId    = RelativeHumidityMeasurement::Attributes::MinMeasuredValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMaxMeasuredValueEntry = {
    .attributeId    = RelativeHumidityMeasurement::Attributes::MaxMeasuredValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kToleranceEntry = {
    .attributeId    = RelativeHumidityMeasurement::Attributes::Tolerance::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace Metadata
} // namespace RelativeHumidityMeasurement
} // namespace clusters
} // namespace app
} // namespace chip
