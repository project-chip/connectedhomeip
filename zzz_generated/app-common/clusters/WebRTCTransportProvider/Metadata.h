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
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(CurrentSessions::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kManage, std::nullopt);
} // namespace CurrentSessions

} // namespace Attributes

namespace Commands {
namespace SolicitOffer {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(SolicitOffer::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kOperate);
} // namespace SolicitOffer
namespace ProvideOffer {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(ProvideOffer::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kOperate);
} // namespace ProvideOffer
namespace ProvideAnswer {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(ProvideAnswer::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kOperate);
} // namespace ProvideAnswer
namespace ProvideICECandidates {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(ProvideICECandidates::Id,
                   BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kOperate);
} // namespace ProvideICECandidates
namespace EndSession {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(EndSession::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kOperate);
} // namespace EndSession

} // namespace Commands
} // namespace WebRTCTransportProvider
} // namespace Clusters
} // namespace app
} // namespace chip
