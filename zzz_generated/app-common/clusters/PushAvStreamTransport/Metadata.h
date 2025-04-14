// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster PushAvStreamTransport (cluster code: 1365/0x555)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/PushAvStreamTransport/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace PushAvStreamTransport {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace SupportedContainerFormats {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::SupportedContainerFormats::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SupportedContainerFormats
namespace SupportedIngestMethods {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::SupportedIngestMethods::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SupportedIngestMethods
namespace CurrentConnections {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::CurrentConnections::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentConnections

} // namespace Attributes

namespace Commands {
namespace AllocatePushTransport {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::AllocatePushTransport::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace AllocatePushTransport
namespace DeallocatePushTransport {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::DeallocatePushTransport::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace DeallocatePushTransport
namespace ModifyPushTransport {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::ModifyPushTransport::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace ModifyPushTransport
namespace SetTransportStatus {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::SetTransportStatus::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace SetTransportStatus
namespace ManuallyTriggerTransport {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::ManuallyTriggerTransport::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace ManuallyTriggerTransport
namespace FindTransport {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::FindTransport::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace FindTransport

} // namespace Commands
} // namespace PushAvStreamTransport
} // namespace Clusters
} // namespace app
} // namespace chip
