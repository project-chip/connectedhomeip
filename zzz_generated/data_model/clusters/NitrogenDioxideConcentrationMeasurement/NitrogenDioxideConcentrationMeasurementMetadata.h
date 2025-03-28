// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster NitrogenDioxideConcentrationMeasurement (cluster code: 1043/0x413)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/NitrogenDioxideConcentrationMeasurement/NitrogenDioxideConcentrationMeasurementIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace NitrogenDioxideConcentrationMeasurement {
namespace Metadata {

inline constexpr uint32_t kRevision = 3;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kMeasuredValueEntry = {
    .attributeId    = NitrogenDioxideConcentrationMeasurement::Attributes::MeasuredValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMinMeasuredValueEntry = {
    .attributeId    = NitrogenDioxideConcentrationMeasurement::Attributes::MinMeasuredValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMaxMeasuredValueEntry = {
    .attributeId    = NitrogenDioxideConcentrationMeasurement::Attributes::MaxMeasuredValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPeakMeasuredValueEntry = {
    .attributeId    = NitrogenDioxideConcentrationMeasurement::Attributes::PeakMeasuredValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPeakMeasuredValueWindowEntry = {
    .attributeId    = NitrogenDioxideConcentrationMeasurement::Attributes::PeakMeasuredValueWindow::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAverageMeasuredValueEntry = {
    .attributeId    = NitrogenDioxideConcentrationMeasurement::Attributes::AverageMeasuredValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAverageMeasuredValueWindowEntry = {
    .attributeId    = NitrogenDioxideConcentrationMeasurement::Attributes::AverageMeasuredValueWindow::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kUncertaintyEntry = {
    .attributeId    = NitrogenDioxideConcentrationMeasurement::Attributes::Uncertainty::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMeasurementUnitEntry = {
    .attributeId    = NitrogenDioxideConcentrationMeasurement::Attributes::MeasurementUnit::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMeasurementMediumEntry = {
    .attributeId    = NitrogenDioxideConcentrationMeasurement::Attributes::MeasurementMedium::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kLevelValueEntry = {
    .attributeId    = NitrogenDioxideConcentrationMeasurement::Attributes::LevelValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace Metadata
} // namespace NitrogenDioxideConcentrationMeasurement
} // namespace clusters
} // namespace app
} // namespace chip
