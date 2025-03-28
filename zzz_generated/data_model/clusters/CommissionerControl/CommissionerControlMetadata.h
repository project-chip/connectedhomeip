// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster CommissionerControl (cluster code: 1873/0x751)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/CommissionerControl/CommissionerControlIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace CommissionerControl {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kSupportedDeviceCategoriesEntry = {
    .attributeId    = CommissionerControl::Attributes::SupportedDeviceCategories::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kRequestCommissioningApprovalEntry = {
    .commandId       = CommissionerControl::Commands::RequestCommissioningApproval::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kCommissionNodeEntry = {
    .commandId       = CommissionerControl::Commands::CommissionNode::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};

} // namespace Commands
} // namespace Metadata
} // namespace CommissionerControl
} // namespace clusters
} // namespace app
} // namespace chip
