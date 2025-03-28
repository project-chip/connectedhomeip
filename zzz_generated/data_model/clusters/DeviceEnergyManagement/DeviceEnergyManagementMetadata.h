// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster DeviceEnergyManagement (cluster code: 152/0x98)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/DeviceEnergyManagement/DeviceEnergyManagementIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace DeviceEnergyManagement {
namespace Metadata {

inline constexpr uint32_t kRevision = 4;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kESATypeEntry = {
    .attributeId    = DeviceEnergyManagement::Attributes::ESAType::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kESACanGenerateEntry = {
    .attributeId    = DeviceEnergyManagement::Attributes::ESACanGenerate::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kESAStateEntry = {
    .attributeId    = DeviceEnergyManagement::Attributes::ESAState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAbsMinPowerEntry = {
    .attributeId    = DeviceEnergyManagement::Attributes::AbsMinPower::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAbsMaxPowerEntry = {
    .attributeId    = DeviceEnergyManagement::Attributes::AbsMaxPower::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPowerAdjustmentCapabilityEntry = {
    .attributeId    = DeviceEnergyManagement::Attributes::PowerAdjustmentCapability::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kForecastEntry = {
    .attributeId    = DeviceEnergyManagement::Attributes::Forecast::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kOptOutStateEntry = {
    .attributeId    = DeviceEnergyManagement::Attributes::OptOutState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kPowerAdjustRequestEntry = {
    .commandId       = DeviceEnergyManagement::Commands::PowerAdjustRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kCancelPowerAdjustRequestEntry = {
    .commandId       = DeviceEnergyManagement::Commands::CancelPowerAdjustRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kStartTimeAdjustRequestEntry = {
    .commandId       = DeviceEnergyManagement::Commands::StartTimeAdjustRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kPauseRequestEntry = {
    .commandId       = DeviceEnergyManagement::Commands::PauseRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kResumeRequestEntry = {
    .commandId       = DeviceEnergyManagement::Commands::ResumeRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kModifyForecastRequestEntry = {
    .commandId       = DeviceEnergyManagement::Commands::ModifyForecastRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kRequestConstraintBasedForecastEntry = {
    .commandId       = DeviceEnergyManagement::Commands::RequestConstraintBasedForecast::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kCancelRequestEntry = {
    .commandId       = DeviceEnergyManagement::Commands::CancelRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace DeviceEnergyManagement
} // namespace clusters
} // namespace app
} // namespace chip
