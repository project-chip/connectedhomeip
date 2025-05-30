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
namespace SupportedFormats {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(SupportedFormats::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace SupportedFormats
namespace CurrentConnections {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(CurrentConnections::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace CurrentConnections

} // namespace Attributes

namespace Commands {
namespace AllocatePushTransport {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(AllocatePushTransport::Id,
                   BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kManage);
} // namespace AllocatePushTransport
namespace DeallocatePushTransport {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(DeallocatePushTransport::Id,
                   BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kManage);
} // namespace DeallocatePushTransport
namespace ModifyPushTransport {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(ModifyPushTransport::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kManage);
} // namespace ModifyPushTransport
namespace SetTransportStatus {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(SetTransportStatus::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kManage);
} // namespace SetTransportStatus
namespace ManuallyTriggerTransport {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(ManuallyTriggerTransport::Id,
                   BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kOperate);
} // namespace ManuallyTriggerTransport
namespace FindTransport {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(FindTransport::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kOperate);
} // namespace FindTransport

} // namespace Commands
} // namespace PushAvStreamTransport
} // namespace Clusters
} // namespace app
} // namespace chip
