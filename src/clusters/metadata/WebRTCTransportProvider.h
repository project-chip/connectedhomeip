// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster WebRTCTransportProvider (cluster code: 1363/0x553)
// based on src/controller/data_model/controller-clusters.matter

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

namespace chip {
namespace app {
namespace clusters {
namespace WebRTCTransportProvider {
namespace Metadata {

inline constexpr ClusterId kClusterId = 0x0553;
inline constexpr uint32_t kRevision   = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kCurrentSessionsEntry = {
    .attributeId    = 0,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kSolicitOfferEntry = {
    .commandId       = 1,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kProvideOfferEntry = {
    .commandId       = 3,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kProvideAnswerEntry = {
    .commandId       = 5,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kProvideICECandidatesEntry = {
    .commandId       = 6,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kEndSessionEntry = {
    .commandId       = 7,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands

} // namespace Metadata
} // namespace WebRTCTransportProvider
} // namespace clusters
} // namespace app
} // namespace chip

// TODO:
//   - help out with mandatory attributes (maybe that array is useful)
//
//   - would probably also want the BUILD.gn file generated to contain all data
//
