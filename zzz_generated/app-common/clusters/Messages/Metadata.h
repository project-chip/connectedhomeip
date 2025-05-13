// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Messages (cluster code: 151/0x97)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/Messages/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Messages {

inline constexpr uint32_t kRevision = 3;

namespace Attributes {
namespace Messages {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(Messages::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace Messages
namespace ActiveMessageIDs {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ActiveMessageIDs::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace ActiveMessageIDs

} // namespace Attributes

namespace Commands {
namespace PresentMessagesRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(PresentMessagesRequest::Id,
                   BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kOperate);
} // namespace PresentMessagesRequest
namespace CancelMessagesRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(CancelMessagesRequest::Id,
                   BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kOperate);
} // namespace CancelMessagesRequest

} // namespace Commands
} // namespace Messages
} // namespace Clusters
} // namespace app
} // namespace chip
