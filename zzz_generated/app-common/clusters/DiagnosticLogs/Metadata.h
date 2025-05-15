// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster DiagnosticLogs (cluster code: 50/0x32)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/DiagnosticLogs/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DiagnosticLogs {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {} // namespace Attributes

namespace Commands {
namespace RetrieveLogsRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(RetrieveLogsRequest::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace RetrieveLogsRequest

} // namespace Commands
} // namespace DiagnosticLogs
} // namespace Clusters
} // namespace app
} // namespace chip
