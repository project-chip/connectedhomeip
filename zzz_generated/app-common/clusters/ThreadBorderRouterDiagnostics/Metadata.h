// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ThreadBorderRouterDiagnostics (cluster code: 1108/0x454)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ThreadBorderRouterDiagnostics/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ThreadBorderRouterDiagnostics {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

namespace Status {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Status::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Status
namespace ActiveFaultsList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ActiveFaultsList::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace ActiveFaultsList
constexpr std::array<DataModel::AttributeEntry, 2> kMandatoryMetadata = {
    Status::kMetadataEntry,
    ActiveFaultsList::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {} // namespace Commands

namespace Events {} // namespace Events
} // namespace ThreadBorderRouterDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
