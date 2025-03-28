// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster WebRTCTransportProvider (cluster code: 1363/0x553)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/WebRTCTransportProvider/WebRTCTransportProviderIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace WebRTCTransportProvider {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kCurrentSessionsEntry = {
    .attributeId    = WebRTCTransportProvider::Attributes::CurrentSessions::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kSolicitOfferEntry = {
    .commandId       = WebRTCTransportProvider::Commands::SolicitOffer::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kProvideOfferEntry = {
    .commandId       = WebRTCTransportProvider::Commands::ProvideOffer::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kProvideAnswerEntry = {
    .commandId       = WebRTCTransportProvider::Commands::ProvideAnswer::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kProvideICECandidatesEntry = {
    .commandId       = WebRTCTransportProvider::Commands::ProvideICECandidates::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kEndSessionEntry = {
    .commandId       = WebRTCTransportProvider::Commands::EndSession::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace WebRTCTransportProvider
} // namespace clusters
} // namespace app
} // namespace chip
