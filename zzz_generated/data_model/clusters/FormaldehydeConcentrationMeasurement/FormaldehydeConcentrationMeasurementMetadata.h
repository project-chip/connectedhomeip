// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster FormaldehydeConcentrationMeasurement (cluster code: 1067/0x42B)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/FormaldehydeConcentrationMeasurement/FormaldehydeConcentrationMeasurementIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace FormaldehydeConcentrationMeasurement {
namespace Metadata {

inline constexpr uint32_t kRevision = 3;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kMeasuredValueEntry = {
    .attributeId    = FormaldehydeConcentrationMeasurement::Attributes::MeasuredValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMinMeasuredValueEntry = {
    .attributeId    = FormaldehydeConcentrationMeasurement::Attributes::MinMeasuredValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMaxMeasuredValueEntry = {
    .attributeId    = FormaldehydeConcentrationMeasurement::Attributes::MaxMeasuredValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPeakMeasuredValueEntry = {
    .attributeId    = FormaldehydeConcentrationMeasurement::Attributes::PeakMeasuredValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPeakMeasuredValueWindowEntry = {
    .attributeId    = FormaldehydeConcentrationMeasurement::Attributes::PeakMeasuredValueWindow::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAverageMeasuredValueEntry = {
    .attributeId    = FormaldehydeConcentrationMeasurement::Attributes::AverageMeasuredValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAverageMeasuredValueWindowEntry = {
    .attributeId    = FormaldehydeConcentrationMeasurement::Attributes::AverageMeasuredValueWindow::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kUncertaintyEntry = {
    .attributeId    = FormaldehydeConcentrationMeasurement::Attributes::Uncertainty::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMeasurementUnitEntry = {
    .attributeId    = FormaldehydeConcentrationMeasurement::Attributes::MeasurementUnit::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMeasurementMediumEntry = {
    .attributeId    = FormaldehydeConcentrationMeasurement::Attributes::MeasurementMedium::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kLevelValueEntry = {
    .attributeId    = FormaldehydeConcentrationMeasurement::Attributes::LevelValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace Metadata
} // namespace FormaldehydeConcentrationMeasurement
} // namespace clusters
} // namespace app
} // namespace chip
