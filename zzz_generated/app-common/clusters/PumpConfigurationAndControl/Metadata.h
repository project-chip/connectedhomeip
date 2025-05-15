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
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxPressure::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxPressure
namespace MaxSpeed {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxSpeed::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxSpeed
namespace MaxFlow {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxFlow::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxFlow
namespace MinConstPressure {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MinConstPressure::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MinConstPressure
namespace MaxConstPressure {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxConstPressure::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxConstPressure
namespace MinCompPressure {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MinCompPressure::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MinCompPressure
namespace MaxCompPressure {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxCompPressure::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxCompPressure
namespace MinConstSpeed {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MinConstSpeed::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MinConstSpeed
namespace MaxConstSpeed {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxConstSpeed::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxConstSpeed
namespace MinConstFlow {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MinConstFlow::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MinConstFlow
namespace MaxConstFlow {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxConstFlow::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxConstFlow
namespace MinConstTemp {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MinConstTemp::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MinConstTemp
namespace MaxConstTemp {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxConstTemp::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxConstTemp
namespace PumpStatus {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PumpStatus::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PumpStatus
namespace EffectiveOperationMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry(EffectiveOperationMode::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace EffectiveOperationMode
namespace EffectiveControlMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry(EffectiveControlMode::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace EffectiveControlMode
namespace Capacity {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Capacity::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Capacity
namespace Speed {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Speed::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Speed
namespace LifetimeRunningHours {
inline constexpr DataModel::AttributeEntry kMetadataEntry(LifetimeRunningHours::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace LifetimeRunningHours
namespace Power {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Power::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Power
namespace LifetimeEnergyConsumed {
inline constexpr DataModel::AttributeEntry kMetadataEntry(LifetimeEnergyConsumed::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace LifetimeEnergyConsumed
namespace OperationMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OperationMode::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace OperationMode
namespace ControlMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ControlMode::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace ControlMode

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace PumpConfigurationAndControl
} // namespace Clusters
} // namespace app
} // namespace chip
