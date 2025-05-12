// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster MeterIdentification (cluster code: 2822/0xB06)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/MeterIdentification/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MeterIdentification {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace MeterType {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MeterType::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MeterType
namespace PointOfDelivery {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PointOfDelivery::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PointOfDelivery
namespace MeterSerialNumber {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MeterSerialNumber::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MeterSerialNumber
namespace ProtocolVersion {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ProtocolVersion::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ProtocolVersion
namespace PowerThreshold {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PowerThreshold::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PowerThreshold

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace MeterIdentification
} // namespace Clusters
} // namespace app
} // namespace chip
