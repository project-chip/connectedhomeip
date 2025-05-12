// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster GroupKeyManagement (cluster code: 63/0x3F)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/GroupKeyManagement/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace GroupKeyManagement {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace GroupKeyMap {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(GroupKeyMap::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, Access::Privilege::kManage);
} // namespace GroupKeyMap
namespace GroupTable {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(GroupTable::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace GroupTable
namespace MaxGroupsPerFabric {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxGroupsPerFabric::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxGroupsPerFabric
namespace MaxGroupKeysPerFabric {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxGroupKeysPerFabric::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxGroupKeysPerFabric

} // namespace Attributes

namespace Commands {
namespace KeySetWrite {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(KeySetWrite::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kAdminister);
} // namespace KeySetWrite
namespace KeySetRead {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(KeySetRead::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kAdminister);
} // namespace KeySetRead
namespace KeySetRemove {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(KeySetRemove::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kAdminister);
} // namespace KeySetRemove
namespace KeySetReadAllIndices {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(KeySetReadAllIndices::Id,
                   BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kAdminister);
} // namespace KeySetReadAllIndices

} // namespace Commands
} // namespace GroupKeyManagement
} // namespace Clusters
} // namespace app
} // namespace chip
