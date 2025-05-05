// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster TlsClientManagement (cluster code: 2050/0x802)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/TlsClientManagement/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace TlsClientManagement {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace MaxProvisioned {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxProvisioned::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxProvisioned
namespace ProvisionedEndpoints {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ProvisionedEndpoints::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace ProvisionedEndpoints

} // namespace Attributes

namespace Commands {
namespace ProvisionEndpoint {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(ProvisionEndpoint::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kAdminister);
} // namespace ProvisionEndpoint
namespace FindEndpoint {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(FindEndpoint::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kOperate);
} // namespace FindEndpoint
namespace RemoveEndpoint {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(RemoveEndpoint::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kAdminister);
} // namespace RemoveEndpoint

} // namespace Commands
} // namespace TlsClientManagement
} // namespace Clusters
} // namespace app
} // namespace chip
