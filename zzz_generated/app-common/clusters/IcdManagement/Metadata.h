// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster IcdManagement (cluster code: 70/0x46)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/IcdManagement/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace IcdManagement {

inline constexpr uint32_t kRevision = 3;

namespace Attributes {
namespace IdleModeDuration {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = IdleModeDuration::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace IdleModeDuration
namespace ActiveModeDuration {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ActiveModeDuration::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ActiveModeDuration
namespace ActiveModeThreshold {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ActiveModeThreshold::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ActiveModeThreshold
namespace RegisteredClients {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RegisteredClients::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace RegisteredClients
namespace ICDCounter {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ICDCounter::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace ICDCounter
namespace ClientsSupportedPerFabric {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ClientsSupportedPerFabric::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ClientsSupportedPerFabric
namespace UserActiveModeTriggerHint {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = UserActiveModeTriggerHint::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace UserActiveModeTriggerHint
namespace UserActiveModeTriggerInstruction {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = UserActiveModeTriggerInstruction::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace UserActiveModeTriggerInstruction
namespace OperatingMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = OperatingMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace OperatingMode
namespace MaximumCheckInBackOff {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MaximumCheckInBackOff::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaximumCheckInBackOff

} // namespace Attributes

namespace Commands {
namespace RegisterClient {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = RegisterClient::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace RegisterClient
namespace UnregisterClient {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = UnregisterClient::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace UnregisterClient
namespace StayActiveRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = StayActiveRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace StayActiveRequest

} // namespace Commands
} // namespace IcdManagement
} // namespace Clusters
} // namespace app
} // namespace chip
