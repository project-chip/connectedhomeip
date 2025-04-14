// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster CommissionerControl (cluster code: 1873/0x751)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/CommissionerControl/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissionerControl {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace SupportedDeviceCategories {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::SupportedDeviceCategories::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = std::nullopt,
};
} // namespace SupportedDeviceCategories

} // namespace Attributes

namespace Commands {
namespace RequestCommissioningApproval {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::RequestCommissioningApproval::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace RequestCommissioningApproval
namespace CommissionNode {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::CommissionNode::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace CommissionNode

} // namespace Commands
} // namespace CommissionerControl
} // namespace Clusters
} // namespace app
} // namespace chip
