// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster WebRTCTransportProvider (cluster code: 1363/0x553)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/WebRTCTransportProvider/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WebRTCTransportProvider {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace CurrentSessions {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::CurrentSessions::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentSessions

} // namespace Attributes

namespace Commands {
namespace SolicitOffer {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::SolicitOffer::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace SolicitOffer
namespace ProvideOffer {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::ProvideOffer::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace ProvideOffer
namespace ProvideAnswer {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::ProvideAnswer::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace ProvideAnswer
namespace ProvideICECandidates {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::ProvideICECandidates::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace ProvideICECandidates
namespace EndSession {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::EndSession::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace EndSession

} // namespace Commands
} // namespace WebRTCTransportProvider
} // namespace Clusters
} // namespace app
} // namespace chip
