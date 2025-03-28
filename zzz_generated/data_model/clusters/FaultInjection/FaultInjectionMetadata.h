// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster FaultInjection (cluster code: 4294048774/0xFFF1FC06)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/FaultInjection/FaultInjectionIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace FaultInjection {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kFailAtFaultEntry = {
    .commandId       = FaultInjection::Commands::FailAtFault::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kFailRandomlyAtFaultEntry = {
    .commandId       = FaultInjection::Commands::FailRandomlyAtFault::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};

} // namespace Commands
} // namespace Metadata
} // namespace FaultInjection
} // namespace clusters
} // namespace app
} // namespace chip
