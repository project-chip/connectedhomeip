// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster PressureMeasurement (cluster code: 1027/0x403)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/PressureMeasurement/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace PressureMeasurement {

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
namespace Tolerance {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Tolerance::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Tolerance
namespace ScaledValue {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ScaledValue::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ScaledValue
namespace MinScaledValue {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MinScaledValue::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MinScaledValue
namespace MaxScaledValue {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxScaledValue::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxScaledValue
namespace ScaledTolerance {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ScaledTolerance::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ScaledTolerance
namespace Scale {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Scale::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Scale

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace PressureMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
