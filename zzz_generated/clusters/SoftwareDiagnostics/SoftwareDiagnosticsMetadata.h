// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster SoftwareDiagnostics (cluster code: 52/0x34)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/SoftwareDiagnostics/SoftwareDiagnosticsIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace SoftwareDiagnostics {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kThreadMetricsEntry = {
    .attributeId    = SoftwareDiagnostics::Attributes::ThreadMetrics::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCurrentHeapFreeEntry = {
    .attributeId    = SoftwareDiagnostics::Attributes::CurrentHeapFree::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCurrentHeapUsedEntry = {
    .attributeId    = SoftwareDiagnostics::Attributes::CurrentHeapUsed::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCurrentHeapHighWatermarkEntry = {
    .attributeId    = SoftwareDiagnostics::Attributes::CurrentHeapHighWatermark::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kResetWatermarksEntry = {
    .commandId       = SoftwareDiagnostics::Commands::ResetWatermarks::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};

} // namespace Commands
} // namespace Metadata
} // namespace SoftwareDiagnostics
} // namespace clusters
} // namespace app
} // namespace chip
