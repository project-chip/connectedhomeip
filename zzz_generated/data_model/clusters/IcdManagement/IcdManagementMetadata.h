// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster IcdManagement (cluster code: 70/0x46)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/IcdManagement/IcdManagementIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace IcdManagement {
namespace Metadata {

inline constexpr uint32_t kRevision = 3;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kIdleModeDurationEntry = {
    .attributeId    = IcdManagement::Attributes::IdleModeDuration::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kActiveModeDurationEntry = {
    .attributeId    = IcdManagement::Attributes::ActiveModeDuration::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kActiveModeThresholdEntry = {
    .attributeId    = IcdManagement::Attributes::ActiveModeThreshold::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kRegisteredClientsEntry = {
    .attributeId    = IcdManagement::Attributes::RegisteredClients::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kICDCounterEntry = {
    .attributeId    = IcdManagement::Attributes::ICDCounter::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kClientsSupportedPerFabricEntry = {
    .attributeId    = IcdManagement::Attributes::ClientsSupportedPerFabric::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kUserActiveModeTriggerHintEntry = {
    .attributeId    = IcdManagement::Attributes::UserActiveModeTriggerHint::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kUserActiveModeTriggerInstructionEntry = {
    .attributeId    = IcdManagement::Attributes::UserActiveModeTriggerInstruction::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kOperatingModeEntry = {
    .attributeId    = IcdManagement::Attributes::OperatingMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMaximumCheckInBackOffEntry = {
    .attributeId    = IcdManagement::Attributes::MaximumCheckInBackOff::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kRegisterClientEntry = {
    .commandId       = IcdManagement::Commands::RegisterClient::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kUnregisterClientEntry = {
    .commandId       = IcdManagement::Commands::UnregisterClient::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kStayActiveRequestEntry = {
    .commandId       = IcdManagement::Commands::StayActiveRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};

} // namespace Commands
} // namespace Metadata
} // namespace IcdManagement
} // namespace clusters
} // namespace app
} // namespace chip
