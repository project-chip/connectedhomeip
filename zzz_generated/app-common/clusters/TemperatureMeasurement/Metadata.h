// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster TemperatureMeasurement (cluster code: 1026/0x402)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
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
constexpr std::array<DataModel::AttributeEntry, 3> kMandatoryMetadata = {
    MeasuredValue::kMetadataEntry,
    MinMeasuredValue::kMetadataEntry,
    MaxMeasuredValue::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {} // namespace Commands

namespace Events {} // namespace Events
} // namespace TemperatureMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
