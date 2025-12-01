// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster SoilMeasurement (cluster code: 1072/0x430)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
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
inline constexpr DataModel::AttributeEntry kMetadataEntry(SoilMoistureMeasurementLimits::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace SoilMoistureMeasurementLimits
namespace SoilMoistureMeasuredValue {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SoilMoistureMeasuredValue::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace SoilMoistureMeasuredValue
constexpr std::array<DataModel::AttributeEntry, 2> kMandatoryMetadata = {
    SoilMoistureMeasurementLimits::kMetadataEntry,
    SoilMoistureMeasuredValue::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {} // namespace Commands

namespace Events {} // namespace Events
} // namespace SoilMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
