// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AdministratorCommissioning (cluster code: 60/0x3C)
// based on src/controller/data_model/controller-clusters.matter

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/AdministratorCommissioning/AdministratorCommissioningIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace AdministratorCommissioning {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kWindowStatusEntry = {
    .attributeId    = Attributes::WindowStatus::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAdminFabricIndexEntry = {
    .attributeId    = Attributes::AdminFabricIndex::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAdminVendorIdEntry = {
    .attributeId    = Attributes::AdminVendorId::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kOpenCommissioningWindowEntry = {
    .commandId       = Commands::OpenCommissioningWindow::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kOpenBasicCommissioningWindowEntry = {
    .commandId       = Commands::OpenBasicCommissioningWindow::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kRevokeCommissioningEntry = {
    .commandId       = Commands::RevokeCommissioning::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kAdminister,
};

} // namespace Commands
} // namespace Metadata
} // namespace AdministratorCommissioning
} // namespace clusters
} // namespace app
} // namespace chip
