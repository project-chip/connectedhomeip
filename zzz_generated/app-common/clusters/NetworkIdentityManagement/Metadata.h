// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster NetworkIdentityManagement (cluster code: 1104/0x450)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/NetworkIdentityManagement/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace NetworkIdentityManagement {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

namespace ActiveNetworkIdentities {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ActiveNetworkIdentities::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kManage, std::nullopt);
} // namespace ActiveNetworkIdentities
namespace Clients {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(Clients::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kManage, std::nullopt);
} // namespace Clients
namespace ClientTableSize {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ClientTableSize::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ClientTableSize
constexpr std::array<DataModel::AttributeEntry, 3> kMandatoryMetadata = {
    ActiveNetworkIdentities::kMetadataEntry,
    Clients::kMetadataEntry,
    ClientTableSize::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {

namespace AddClient {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(AddClient::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kManage);
} // namespace AddClient
namespace RemoveClient {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(RemoveClient::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kManage);
} // namespace RemoveClient
namespace QueryIdentity {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(QueryIdentity::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kManage);
} // namespace QueryIdentity
namespace ImportAdminSecret {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(ImportAdminSecret::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kAdminister);
} // namespace ImportAdminSecret
namespace ExportAdminSecret {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ExportAdminSecret::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace ExportAdminSecret

} // namespace Commands

namespace Events {} // namespace Events
} // namespace NetworkIdentityManagement
} // namespace Clusters
} // namespace app
} // namespace chip
