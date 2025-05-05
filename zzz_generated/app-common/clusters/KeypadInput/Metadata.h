// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster KeypadInput (cluster code: 1289/0x509)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/KeypadInput/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace KeypadInput {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {} // namespace Attributes

namespace Commands {
namespace SendKey {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(SendKey::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace SendKey

} // namespace Commands
} // namespace KeypadInput
} // namespace Clusters
} // namespace app
} // namespace chip
