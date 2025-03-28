// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster PushAvStreamTransport (cluster code: 1365/0x555)
// based on src/controller/data_model/controller-clusters.matter

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/PushAvStreamTransport/PushAvStreamTransportIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace PushAvStreamTransport {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kSupportedContainerFormatsEntry = {
    .attributeId    = Attributes::SupportedContainerFormats::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSupportedIngestMethodsEntry = {
    .attributeId    = Attributes::SupportedIngestMethods::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCurrentConnectionsEntry = {
    .attributeId    = Attributes::CurrentConnections::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kAllocatePushTransportEntry = {
    .commandId       = Commands::AllocatePushTransport::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kDeallocatePushTransportEntry = {
    .commandId       = Commands::DeallocatePushTransport::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kModifyPushTransportEntry = {
    .commandId       = Commands::ModifyPushTransport::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kSetTransportStatusEntry = {
    .commandId       = Commands::SetTransportStatus::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kManuallyTriggerTransportEntry = {
    .commandId       = Commands::ManuallyTriggerTransport::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kFindTransportEntry = {
    .commandId       = Commands::FindTransport::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace PushAvStreamTransport
} // namespace clusters
} // namespace app
} // namespace chip
