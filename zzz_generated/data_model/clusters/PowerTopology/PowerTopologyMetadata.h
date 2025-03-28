// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster PowerTopology (cluster code: 156/0x9C)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/PowerTopology/PowerTopologyIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace PowerTopology {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kAvailableEndpointsEntry = {
    .attributeId    = PowerTopology::Attributes::AvailableEndpoints::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kActiveEndpointsEntry = {
    .attributeId    = PowerTopology::Attributes::ActiveEndpoints::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace Metadata
} // namespace PowerTopology
} // namespace clusters
} // namespace app
} // namespace chip
