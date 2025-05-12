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
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ThreadMetrics::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace ThreadMetrics
namespace CurrentHeapFree {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentHeapFree::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentHeapFree
namespace CurrentHeapUsed {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentHeapUsed::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentHeapUsed
namespace CurrentHeapHighWatermark {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentHeapHighWatermark::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace CurrentHeapHighWatermark

} // namespace Attributes

namespace Commands {
namespace ResetWatermarks {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ResetWatermarks::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kManage);
} // namespace ResetWatermarks

} // namespace Commands
} // namespace SoftwareDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
