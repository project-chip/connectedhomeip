// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster FaultInjection (cluster code: 4294048774/0xFFF1FC06)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/FaultInjection/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace FaultInjection {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
inline constexpr uint32_t kMetadataCount = 5;

} // namespace Attributes

namespace Commands {
inline constexpr uint32_t kMetadataCount = 2;

namespace FailAtFault {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(FailAtFault::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kManage);
} // namespace FailAtFault
namespace FailRandomlyAtFault {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(FailRandomlyAtFault::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kManage);
} // namespace FailRandomlyAtFault

} // namespace Commands

namespace Events {} // namespace Events
} // namespace FaultInjection
} // namespace Clusters
} // namespace app
} // namespace chip
