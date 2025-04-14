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
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::AcceptHeader::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace AcceptHeader
namespace SupportedStreamingProtocols {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::SupportedStreamingProtocols::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SupportedStreamingProtocols

} // namespace Attributes

namespace Commands {
namespace LaunchContent {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::LaunchContent::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace LaunchContent
namespace LaunchURL {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::LaunchURL::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace LaunchURL

} // namespace Commands
} // namespace ContentLauncher
} // namespace Clusters
} // namespace app
} // namespace chip
