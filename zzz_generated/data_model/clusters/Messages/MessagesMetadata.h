// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Messages (cluster code: 151/0x97)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/Messages/MessagesIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace Messages {
namespace Metadata {

inline constexpr uint32_t kRevision = 3;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kMessagesEntry = {
    .attributeId    = Messages::Attributes::Messages::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kActiveMessageIDsEntry = {
    .attributeId    = Messages::Attributes::ActiveMessageIDs::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kPresentMessagesRequestEntry = {
    .commandId       = Messages::Commands::PresentMessagesRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kCancelMessagesRequestEntry = {
    .commandId       = Messages::Commands::CancelMessagesRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace Messages
} // namespace clusters
} // namespace app
} // namespace chip
