// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster OtaSoftwareUpdateProvider (cluster code: 41/0x29)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/OtaSoftwareUpdateProvider/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OtaSoftwareUpdateProvider {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {} // namespace Attributes

namespace Commands {
namespace QueryImage {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(QueryImage::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace QueryImage
namespace ApplyUpdateRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ApplyUpdateRequest::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace ApplyUpdateRequest
namespace NotifyUpdateApplied {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(NotifyUpdateApplied::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace NotifyUpdateApplied

} // namespace Commands
} // namespace OtaSoftwareUpdateProvider
} // namespace Clusters
} // namespace app
} // namespace chip
