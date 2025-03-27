// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster TlsClientManagement (cluster code: 2050/0x802)
// based on src/controller/data_model/controller-clusters.matter

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

namespace chip {
namespace app {
namespace clusters {
namespace TlsClientManagement {
namespace Metadata {

inline constexpr ClusterId kClusterId = 0x0802;
inline constexpr uint32_t kRevision   = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kMaxProvisionedEntry = {
    .attributeId    = 0,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kProvisionedEndpointsEntry = {
    .attributeId    = 1,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kProvisionEndpointEntry = {
    .commandId       = 0,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kFindEndpointEntry = {
    .commandId       = 2,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kRemoveEndpointEntry = {
    .commandId       = 4,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};

} // namespace Commands

} // namespace Metadata
} // namespace TlsClientManagement
} // namespace clusters
} // namespace app
} // namespace chip
