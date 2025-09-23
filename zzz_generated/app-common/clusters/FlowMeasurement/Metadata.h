// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster FlowMeasurement (cluster code: 1028/0x404)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/FlowMeasurement/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace FlowMeasurement {

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
constexpr std::array<DataModel::AttributeEntry, 3> kMandatoryMetadata = {
    MeasuredValue::kMetadataEntry,
    MinMeasuredValue::kMetadataEntry,
    MaxMeasuredValue::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {} // namespace Commands

namespace Events {} // namespace Events
} // namespace FlowMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
