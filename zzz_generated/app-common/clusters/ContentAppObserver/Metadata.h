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

namespace Attributes {
inline constexpr uint32_t kMetadataCount = 5;

} // namespace Attributes

namespace Commands {
inline constexpr uint32_t kMetadataCount = 1;

namespace ContentAppMessage {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ContentAppMessage::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace ContentAppMessage

} // namespace Commands
} // namespace ContentAppObserver
} // namespace Clusters
} // namespace app
} // namespace chip
