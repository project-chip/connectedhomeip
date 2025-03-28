// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster OtaSoftwareUpdateProvider (cluster code: 41/0x29)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/OtaSoftwareUpdateProvider/OtaSoftwareUpdateProviderIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace OtaSoftwareUpdateProvider {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kQueryImageEntry = {
    .commandId       = OtaSoftwareUpdateProvider::Commands::QueryImage::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kApplyUpdateRequestEntry = {
    .commandId       = OtaSoftwareUpdateProvider::Commands::ApplyUpdateRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kNotifyUpdateAppliedEntry = {
    .commandId       = OtaSoftwareUpdateProvider::Commands::NotifyUpdateApplied::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace OtaSoftwareUpdateProvider
} // namespace clusters
} // namespace app
} // namespace chip
