// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster RadonConcentrationMeasurement (cluster code: 1071/0x42F)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/RadonConcentrationMeasurement/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace RadonConcentrationMeasurement {

inline constexpr uint32_t kRevision = 3;

namespace Attributes {
namespace MeasuredValue {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RadonConcentrationMeasurement::Attributes::MeasuredValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MeasuredValue
namespace MinMeasuredValue {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RadonConcentrationMeasurement::Attributes::MinMeasuredValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MinMeasuredValue
namespace MaxMeasuredValue {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RadonConcentrationMeasurement::Attributes::MaxMeasuredValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxMeasuredValue
namespace PeakMeasuredValue {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RadonConcentrationMeasurement::Attributes::PeakMeasuredValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PeakMeasuredValue
namespace PeakMeasuredValueWindow {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RadonConcentrationMeasurement::Attributes::PeakMeasuredValueWindow::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PeakMeasuredValueWindow
namespace AverageMeasuredValue {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RadonConcentrationMeasurement::Attributes::AverageMeasuredValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace AverageMeasuredValue
namespace AverageMeasuredValueWindow {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RadonConcentrationMeasurement::Attributes::AverageMeasuredValueWindow::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace AverageMeasuredValueWindow
namespace Uncertainty {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RadonConcentrationMeasurement::Attributes::Uncertainty::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Uncertainty
namespace MeasurementUnit {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RadonConcentrationMeasurement::Attributes::MeasurementUnit::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MeasurementUnit
namespace MeasurementMedium {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RadonConcentrationMeasurement::Attributes::MeasurementMedium::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MeasurementMedium
namespace LevelValue {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RadonConcentrationMeasurement::Attributes::LevelValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace LevelValue

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace RadonConcentrationMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
