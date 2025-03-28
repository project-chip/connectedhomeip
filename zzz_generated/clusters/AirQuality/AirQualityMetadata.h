// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AirQuality (cluster code: 91/0x5B)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/AirQuality/AirQualityIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace AirQuality {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kAirQualityEntry = {
    .attributeId    = AirQuality::Attributes::AirQuality::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace Metadata
} // namespace AirQuality
} // namespace clusters
} // namespace app
} // namespace chip
