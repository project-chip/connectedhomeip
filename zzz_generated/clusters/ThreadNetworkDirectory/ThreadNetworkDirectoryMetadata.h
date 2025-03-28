// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ThreadNetworkDirectory (cluster code: 1107/0x453)
// based on src/controller/data_model/controller-clusters.matter

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

namespace chip {
namespace app {
namespace clusters {
namespace ThreadNetworkDirectory {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kPreferredExtendedPanIDEntry = {
    .attributeId    = 0,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kThreadNetworksEntry = {
    .attributeId    = 1,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kOperate,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kThreadNetworkTableSizeEntry = {
    .attributeId    = 2,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kAddNetworkEntry = {
    .commandId       = 0,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kRemoveNetworkEntry = {
    .commandId       = 1,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kGetOperationalDatasetEntry = {
    .commandId       = 2,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace ThreadNetworkDirectory
} // namespace clusters
} // namespace app
} // namespace chip
