// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AdministratorCommissioning (cluster code: 60/0x3C)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/AdministratorCommissioning/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AdministratorCommissioning {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace WindowStatus {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::WindowStatus::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace WindowStatus
namespace AdminFabricIndex {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::AdminFabricIndex::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace AdminFabricIndex
namespace AdminVendorId {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::AdminVendorId::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace AdminVendorId

} // namespace Attributes

namespace Commands {
namespace OpenCommissioningWindow {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::OpenCommissioningWindow::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace OpenCommissioningWindow
namespace OpenBasicCommissioningWindow {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::OpenBasicCommissioningWindow::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace OpenBasicCommissioningWindow
namespace RevokeCommissioning {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::RevokeCommissioning::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace RevokeCommissioning

} // namespace Commands
} // namespace AdministratorCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip
