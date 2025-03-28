// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster TlsClientManagement (cluster code: 2050/0x802)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/TlsClientManagement/TlsClientManagementIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace TlsClientManagement {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kMaxProvisionedEntry = {
    .attributeId    = TlsClientManagement::Attributes::MaxProvisioned::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kProvisionedEndpointsEntry = {
    .attributeId    = TlsClientManagement::Attributes::ProvisionedEndpoints::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kProvisionEndpointEntry = {
    .commandId       = TlsClientManagement::Commands::ProvisionEndpoint::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kFindEndpointEntry = {
    .commandId       = TlsClientManagement::Commands::FindEndpoint::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kRemoveEndpointEntry = {
    .commandId       = TlsClientManagement::Commands::RemoveEndpoint::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};

} // namespace Commands
} // namespace Metadata
} // namespace TlsClientManagement
} // namespace clusters
} // namespace app
} // namespace chip
