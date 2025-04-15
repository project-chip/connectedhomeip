// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster TotalVolatileOrganicCompoundsConcentrationMeasurement (cluster code: 1070/0x42E)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/TotalVolatileOrganicCompoundsConcentrationMeasurement/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace TotalVolatileOrganicCompoundsConcentrationMeasurement {

inline constexpr uint32_t kRevision = 3;

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
namespace PeakMeasuredValue {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PeakMeasuredValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PeakMeasuredValue
namespace PeakMeasuredValueWindow {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PeakMeasuredValueWindow::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PeakMeasuredValueWindow
namespace AverageMeasuredValue {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AverageMeasuredValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace AverageMeasuredValue
namespace AverageMeasuredValueWindow {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AverageMeasuredValueWindow::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace AverageMeasuredValueWindow
namespace Uncertainty {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Uncertainty::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Uncertainty
namespace MeasurementUnit {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MeasurementUnit::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MeasurementUnit
namespace MeasurementMedium {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MeasurementMedium::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MeasurementMedium
namespace LevelValue {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LevelValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace LevelValue

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace TotalVolatileOrganicCompoundsConcentrationMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
