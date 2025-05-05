// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Descriptor (cluster code: 29/0x1D)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/Descriptor/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Descriptor {

inline constexpr uint32_t kRevision = 3;

namespace Attributes {
namespace DeviceTypeList {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = DeviceTypeList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace DeviceTypeList
namespace ServerList {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ServerList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ServerList
namespace ClientList {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ClientList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ClientList
namespace PartsList {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PartsList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PartsList
namespace TagList {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TagList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TagList
namespace EndpointUniqueID {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EndpointUniqueID::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace EndpointUniqueID

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace Descriptor
} // namespace Clusters
} // namespace app
} // namespace chip
