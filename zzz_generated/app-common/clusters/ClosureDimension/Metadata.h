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
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CurrentState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentState
namespace Target {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Target::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Target
namespace Resolution {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Resolution::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Resolution
namespace StepValue {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = StepValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace StepValue
namespace Unit {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Unit::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Unit
namespace UnitRange {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = UnitRange::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace UnitRange
namespace LimitRange {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LimitRange::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace LimitRange
namespace TranslationDirection {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TranslationDirection::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TranslationDirection
namespace RotationAxis {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RotationAxis::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RotationAxis
namespace Overflow {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Overflow::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Overflow
namespace ModulationType {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ModulationType::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ModulationType

} // namespace Attributes

namespace Commands {
namespace SetTarget {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = SetTarget::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace SetTarget
namespace Step {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Step::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace Step

} // namespace Commands
} // namespace ClosureDimension
} // namespace Clusters
} // namespace app
} // namespace chip
