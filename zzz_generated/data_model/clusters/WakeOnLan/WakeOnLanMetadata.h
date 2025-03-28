// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster WakeOnLan (cluster code: 1283/0x503)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/WakeOnLan/WakeOnLanIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace WakeOnLan {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kMACAddressEntry = {
    .attributeId    = WakeOnLan::Attributes::MACAddress::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kLinkLocalAddressEntry = {
    .attributeId    = WakeOnLan::Attributes::LinkLocalAddress::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace Metadata
} // namespace WakeOnLan
} // namespace clusters
} // namespace app
} // namespace chip
