// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ContentAppObserver (cluster code: 1296/0x510)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ContentAppObserver/ContentAppObserverIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace ContentAppObserver {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kContentAppMessageEntry = {
    .commandId       = ContentAppObserver::Commands::ContentAppMessage::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace ContentAppObserver
} // namespace clusters
} // namespace app
} // namespace chip
