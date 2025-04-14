// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster PumpConfigurationAndControl (cluster code: 512/0x200)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/PumpConfigurationAndControl/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace PumpConfigurationAndControl {

inline constexpr uint32_t kRevision = 4;

namespace Attributes {
namespace MaxPressure {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::MaxPressure::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxPressure
namespace MaxSpeed {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::MaxSpeed::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxSpeed
namespace MaxFlow {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::MaxFlow::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxFlow
namespace MinConstPressure {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::MinConstPressure::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MinConstPressure
namespace MaxConstPressure {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::MaxConstPressure::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxConstPressure
namespace MinCompPressure {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::MinCompPressure::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MinCompPressure
namespace MaxCompPressure {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::MaxCompPressure::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxCompPressure
namespace MinConstSpeed {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::MinConstSpeed::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MinConstSpeed
namespace MaxConstSpeed {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::MaxConstSpeed::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxConstSpeed
namespace MinConstFlow {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::MinConstFlow::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MinConstFlow
namespace MaxConstFlow {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::MaxConstFlow::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxConstFlow
namespace MinConstTemp {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::MinConstTemp::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MinConstTemp
namespace MaxConstTemp {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::MaxConstTemp::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxConstTemp
namespace PumpStatus {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::PumpStatus::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PumpStatus
namespace EffectiveOperationMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::EffectiveOperationMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace EffectiveOperationMode
namespace EffectiveControlMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::EffectiveControlMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace EffectiveControlMode
namespace Capacity {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::Capacity::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Capacity
namespace Speed {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::Speed::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Speed
namespace LifetimeRunningHours {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::LifetimeRunningHours::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace LifetimeRunningHours
namespace Power {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::Power::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Power
namespace LifetimeEnergyConsumed {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::LifetimeEnergyConsumed::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace LifetimeEnergyConsumed
namespace OperationMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::OperationMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace OperationMode
namespace ControlMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::ControlMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace ControlMode

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace PumpConfigurationAndControl
} // namespace Clusters
} // namespace app
} // namespace chip
