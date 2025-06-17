// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ContentLauncher (cluster code: 1290/0x50A)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ContentLauncher/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ContentLauncher {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace AcceptHeader {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(AcceptHeader::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace AcceptHeader
namespace SupportedStreamingProtocols {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SupportedStreamingProtocols::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace SupportedStreamingProtocols

} // namespace Attributes

namespace Commands {
namespace LaunchContent {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(LaunchContent::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace LaunchContent
namespace LaunchURL {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(LaunchURL::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace LaunchURL

} // namespace Commands
} // namespace ContentLauncher
} // namespace Clusters
} // namespace app
} // namespace chip
