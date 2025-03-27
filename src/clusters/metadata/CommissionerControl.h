// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster CommissionerControl (cluster code: 1873/0x751)
// based on src/controller/data_model/controller-clusters.matter

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

namespace chip {
namespace app {
namespace clusters {
namespace CommissionerControl {
namespace Metadata {

inline constexpr ClusterId kClusterId = 0x0751;
inline constexpr uint32_t kRevision   = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kSupportedDeviceCategoriesEntry = {
    .attributeId    = 0,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kRequestCommissioningApprovalEntry = {
    .commandId       = 0,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kCommissionNodeEntry = {
    .commandId       = 1,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};

} // namespace Commands

} // namespace Metadata
} // namespace CommissionerControl
} // namespace clusters
} // namespace app
} // namespace chip

// TODO:
//   - help out with mandatory attributes (maybe that array is useful)
//
//   - would probably also want the BUILD.gn file generated to contain all data
//
