// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster OtaSoftwareUpdateRequestor (cluster code: 42/0x2A)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/OtaSoftwareUpdateRequestor/OtaSoftwareUpdateRequestorIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace OtaSoftwareUpdateRequestor {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kDefaultOTAProvidersEntry = {
    .attributeId    = OtaSoftwareUpdateRequestor::Attributes::DefaultOTAProviders::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AttributeEntry kUpdatePossibleEntry = {
    .attributeId    = OtaSoftwareUpdateRequestor::Attributes::UpdatePossible::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kUpdateStateEntry = {
    .attributeId    = OtaSoftwareUpdateRequestor::Attributes::UpdateState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kUpdateStateProgressEntry = {
    .attributeId    = OtaSoftwareUpdateRequestor::Attributes::UpdateStateProgress::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kAnnounceOTAProviderEntry = {
    .commandId       = OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace OtaSoftwareUpdateRequestor
} // namespace clusters
} // namespace app
} // namespace chip
