// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster SoftwareDiagnostics (cluster code: 52/0x34)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/SoftwareDiagnostics/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace SoftwareDiagnostics {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace ThreadMetrics {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::ThreadMetrics::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ThreadMetrics
namespace CurrentHeapFree {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::CurrentHeapFree::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentHeapFree
namespace CurrentHeapUsed {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::CurrentHeapUsed::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentHeapUsed
namespace CurrentHeapHighWatermark {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::CurrentHeapHighWatermark::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentHeapHighWatermark

} // namespace Attributes

namespace Commands {
namespace ResetWatermarks {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::ResetWatermarks::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace ResetWatermarks

} // namespace Commands
} // namespace SoftwareDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
