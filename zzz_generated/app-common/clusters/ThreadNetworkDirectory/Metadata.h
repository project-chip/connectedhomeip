// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ThreadNetworkDirectory (cluster code: 1107/0x453)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ThreadNetworkDirectory/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ThreadNetworkDirectory {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace PreferredExtendedPanID {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PreferredExtendedPanID::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace PreferredExtendedPanID
namespace ThreadNetworks {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ThreadNetworks::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace ThreadNetworks
namespace ThreadNetworkTableSize {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ThreadNetworkTableSize::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ThreadNetworkTableSize

} // namespace Attributes

namespace Commands {
namespace AddNetwork {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(AddNetwork::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kManage);
} // namespace AddNetwork
namespace RemoveNetwork {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(RemoveNetwork::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kManage);
} // namespace RemoveNetwork
namespace GetOperationalDataset {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(GetOperationalDataset::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kManage);
} // namespace GetOperationalDataset

} // namespace Commands
} // namespace ThreadNetworkDirectory
} // namespace Clusters
} // namespace app
} // namespace chip
