// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ClosureDimension (cluster code: 261/0x105)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ClosureDimension/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureDimension {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace CurrentState {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentState::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentState
namespace TargetState {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TargetState::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TargetState
namespace Resolution {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Resolution::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Resolution
namespace StepValue {
inline constexpr DataModel::AttributeEntry kMetadataEntry(StepValue::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace StepValue
namespace Unit {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Unit::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Unit
namespace UnitRange {
inline constexpr DataModel::AttributeEntry kMetadataEntry(UnitRange::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace UnitRange
namespace LimitRange {
inline constexpr DataModel::AttributeEntry kMetadataEntry(LimitRange::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace LimitRange
namespace TranslationDirection {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TranslationDirection::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TranslationDirection
namespace RotationAxis {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RotationAxis::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace RotationAxis
namespace Overflow {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Overflow::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Overflow
namespace ModulationType {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ModulationType::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ModulationType
namespace LatchControlModes {
inline constexpr DataModel::AttributeEntry kMetadataEntry(LatchControlModes::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace LatchControlModes

} // namespace Attributes

namespace Commands {
namespace SetTarget {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(SetTarget::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kOperate);
} // namespace SetTarget
namespace Step {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(Step::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kOperate);
} // namespace Step

} // namespace Commands
} // namespace ClosureDimension
} // namespace Clusters
} // namespace app
} // namespace chip
