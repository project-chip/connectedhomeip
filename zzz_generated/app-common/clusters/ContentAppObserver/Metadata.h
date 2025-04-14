// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ContentAppObserver (cluster code: 1296/0x510)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ContentAppObserver/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ContentAppObserver {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {} // namespace Attributes

namespace Commands {
namespace ContentAppMessage {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::ContentAppMessage::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace ContentAppMessage

} // namespace Commands
} // namespace ContentAppObserver
} // namespace Clusters
} // namespace app
} // namespace chip
