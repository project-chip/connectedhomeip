// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster PumpConfigurationAndControl (cluster code: 512/0x200)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/PumpConfigurationAndControl/PumpConfigurationAndControlIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace PumpConfigurationAndControl {
namespace Metadata {

inline constexpr uint32_t kRevision = 4;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kMaxPressureEntry = {
    .attributeId    = PumpConfigurationAndControl::Attributes::MaxPressure::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMaxSpeedEntry = {
    .attributeId    = PumpConfigurationAndControl::Attributes::MaxSpeed::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMaxFlowEntry = {
    .attributeId    = PumpConfigurationAndControl::Attributes::MaxFlow::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMinConstPressureEntry = {
    .attributeId    = PumpConfigurationAndControl::Attributes::MinConstPressure::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMaxConstPressureEntry = {
    .attributeId    = PumpConfigurationAndControl::Attributes::MaxConstPressure::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMinCompPressureEntry = {
    .attributeId    = PumpConfigurationAndControl::Attributes::MinCompPressure::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMaxCompPressureEntry = {
    .attributeId    = PumpConfigurationAndControl::Attributes::MaxCompPressure::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMinConstSpeedEntry = {
    .attributeId    = PumpConfigurationAndControl::Attributes::MinConstSpeed::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMaxConstSpeedEntry = {
    .attributeId    = PumpConfigurationAndControl::Attributes::MaxConstSpeed::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMinConstFlowEntry = {
    .attributeId    = PumpConfigurationAndControl::Attributes::MinConstFlow::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMaxConstFlowEntry = {
    .attributeId    = PumpConfigurationAndControl::Attributes::MaxConstFlow::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMinConstTempEntry = {
    .attributeId    = PumpConfigurationAndControl::Attributes::MinConstTemp::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMaxConstTempEntry = {
    .attributeId    = PumpConfigurationAndControl::Attributes::MaxConstTemp::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPumpStatusEntry = {
    .attributeId    = PumpConfigurationAndControl::Attributes::PumpStatus::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kEffectiveOperationModeEntry = {
    .attributeId    = PumpConfigurationAndControl::Attributes::EffectiveOperationMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kEffectiveControlModeEntry = {
    .attributeId    = PumpConfigurationAndControl::Attributes::EffectiveControlMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCapacityEntry = {
    .attributeId    = PumpConfigurationAndControl::Attributes::Capacity::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSpeedEntry = {
    .attributeId    = PumpConfigurationAndControl::Attributes::Speed::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kLifetimeRunningHoursEntry = {
    .attributeId    = PumpConfigurationAndControl::Attributes::LifetimeRunningHours::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kPowerEntry = {
    .attributeId    = PumpConfigurationAndControl::Attributes::Power::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kLifetimeEnergyConsumedEntry = {
    .attributeId    = PumpConfigurationAndControl::Attributes::LifetimeEnergyConsumed::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kOperationModeEntry = {
    .attributeId    = PumpConfigurationAndControl::Attributes::OperationMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kControlModeEntry = {
    .attributeId    = PumpConfigurationAndControl::Attributes::ControlMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace Metadata
} // namespace PumpConfigurationAndControl
} // namespace clusters
} // namespace app
} // namespace chip
