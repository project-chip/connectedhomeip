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
inline constexpr DataModel::AttributeEntry kMetadataEntry(SupportedDeviceCategories::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kManage,
                                                          std::nullopt);
} // namespace SupportedDeviceCategories

} // namespace Attributes

namespace Commands {
namespace RequestCommissioningApproval {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(RequestCommissioningApproval::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kManage);
} // namespace RequestCommissioningApproval
namespace CommissionNode {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(CommissionNode::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kManage);
} // namespace CommissionNode

} // namespace Commands
} // namespace CommissionerControl
} // namespace Clusters
} // namespace app
} // namespace chip
