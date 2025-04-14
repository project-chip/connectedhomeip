// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster OtaSoftwareUpdateRequestor (cluster code: 42/0x2A)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/OtaSoftwareUpdateRequestor/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OtaSoftwareUpdateRequestor {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace DefaultOTAProviders {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::DefaultOTAProviders::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kAdminister,
};
} // namespace DefaultOTAProviders
namespace UpdatePossible {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::UpdatePossible::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace UpdatePossible
namespace UpdateState {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::UpdateState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace UpdateState
namespace UpdateStateProgress {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::UpdateStateProgress::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace UpdateStateProgress

} // namespace Attributes

namespace Commands {
namespace AnnounceOTAProvider {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::AnnounceOTAProvider::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace AnnounceOTAProvider

} // namespace Commands
} // namespace OtaSoftwareUpdateRequestor
} // namespace Clusters
} // namespace app
} // namespace chip
