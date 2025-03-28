// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster DiagnosticLogs (cluster code: 50/0x32)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/DiagnosticLogs/DiagnosticLogsIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace DiagnosticLogs {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kRetrieveLogsRequestEntry = {
    .commandId       = DiagnosticLogs::Commands::RetrieveLogsRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace DiagnosticLogs
} // namespace clusters
} // namespace app
} // namespace chip
