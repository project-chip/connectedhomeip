// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster WakeOnLan (cluster code: 1283/0x503)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/WakeOnLan/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WakeOnLan {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace MACAddress {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::MACAddress::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MACAddress
namespace LinkLocalAddress {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::LinkLocalAddress::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace LinkLocalAddress

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace WakeOnLan
} // namespace Clusters
} // namespace app
} // namespace chip
