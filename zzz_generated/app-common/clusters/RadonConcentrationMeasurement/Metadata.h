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
inline constexpr DataModel::AttributeEntry kMetadataEntry(MeasuredValue::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MeasuredValue
namespace MinMeasuredValue {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MinMeasuredValue::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MinMeasuredValue
namespace MaxMeasuredValue {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxMeasuredValue::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxMeasuredValue
namespace PeakMeasuredValue {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PeakMeasuredValue::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PeakMeasuredValue
namespace PeakMeasuredValueWindow {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PeakMeasuredValueWindow::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PeakMeasuredValueWindow
namespace AverageMeasuredValue {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AverageMeasuredValue::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace AverageMeasuredValue
namespace AverageMeasuredValueWindow {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AverageMeasuredValueWindow::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AverageMeasuredValueWindow
namespace Uncertainty {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Uncertainty::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Uncertainty
namespace MeasurementUnit {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MeasurementUnit::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MeasurementUnit
namespace MeasurementMedium {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MeasurementMedium::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MeasurementMedium
namespace LevelValue {
inline constexpr DataModel::AttributeEntry kMetadataEntry(LevelValue::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace LevelValue

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace RadonConcentrationMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
