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
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(DeviceTypeList::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace DeviceTypeList
namespace ServerList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ServerList::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace ServerList
namespace ClientList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ClientList::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace ClientList
namespace PartsList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(PartsList::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace PartsList
namespace TagList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(TagList::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace TagList
namespace EndpointUniqueID {
inline constexpr DataModel::AttributeEntry kMetadataEntry(EndpointUniqueID::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace EndpointUniqueID

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace Descriptor
} // namespace Clusters
} // namespace app
} // namespace chip
