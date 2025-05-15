// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster WebRTCTransportRequestor (cluster code: 1364/0x554)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/WebRTCTransportRequestor/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WebRTCTransportRequestor {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace CurrentSessions {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(CurrentSessions::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kAdminister, std::nullopt);
} // namespace CurrentSessions

} // namespace Attributes

namespace Commands {
namespace Offer {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Offer::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace Offer
namespace Answer {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Answer::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace Answer
namespace ICECandidates {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ICECandidates::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace ICECandidates
namespace End {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(End::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace End

} // namespace Commands
} // namespace WebRTCTransportRequestor
} // namespace Clusters
} // namespace app
} // namespace chip
