// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster WebRTCTransportRequestor (cluster code: 1364/0x554)
// based on src/controller/data_model/controller-clusters.matter

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/WebRTCTransportRequestor/WebRTCTransportRequestorIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace WebRTCTransportRequestor {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kCurrentSessionsEntry = {
    .attributeId    = Attributes::CurrentSessions::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kOfferEntry = {
    .commandId       = Commands::Offer::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kAnswerEntry = {
    .commandId       = Commands::Answer::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kICECandidatesEntry = {
    .commandId       = Commands::ICECandidates::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kEndEntry = {
    .commandId       = Commands::End::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace WebRTCTransportRequestor
} // namespace clusters
} // namespace app
} // namespace chip
