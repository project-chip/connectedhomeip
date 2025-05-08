// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster WindowCovering (cluster code: 258/0x102)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/WindowCovering/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WindowCovering {

inline constexpr uint32_t kRevision = 5;

namespace Attributes {
namespace Type {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Type::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Type
namespace PhysicalClosedLimitLift {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PhysicalClosedLimitLift::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PhysicalClosedLimitLift
namespace PhysicalClosedLimitTilt {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PhysicalClosedLimitTilt::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PhysicalClosedLimitTilt
namespace CurrentPositionLift {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CurrentPositionLift::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentPositionLift
namespace CurrentPositionTilt {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CurrentPositionTilt::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentPositionTilt
namespace NumberOfActuationsLift {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NumberOfActuationsLift::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NumberOfActuationsLift
namespace NumberOfActuationsTilt {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NumberOfActuationsTilt::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NumberOfActuationsTilt
namespace ConfigStatus {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ConfigStatus::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ConfigStatus
namespace CurrentPositionLiftPercentage {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CurrentPositionLiftPercentage::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentPositionLiftPercentage
namespace CurrentPositionTiltPercentage {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CurrentPositionTiltPercentage::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentPositionTiltPercentage
namespace OperationalStatus {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = OperationalStatus::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace OperationalStatus
namespace TargetPositionLiftPercent100ths {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TargetPositionLiftPercent100ths::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TargetPositionLiftPercent100ths
namespace TargetPositionTiltPercent100ths {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TargetPositionTiltPercent100ths::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TargetPositionTiltPercent100ths
namespace EndProductType {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EndProductType::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace EndProductType
namespace CurrentPositionLiftPercent100ths {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CurrentPositionLiftPercent100ths::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentPositionLiftPercent100ths
namespace CurrentPositionTiltPercent100ths {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CurrentPositionTiltPercent100ths::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentPositionTiltPercent100ths
namespace InstalledOpenLimitLift {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = InstalledOpenLimitLift::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace InstalledOpenLimitLift
namespace InstalledClosedLimitLift {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = InstalledClosedLimitLift::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace InstalledClosedLimitLift
namespace InstalledOpenLimitTilt {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = InstalledOpenLimitTilt::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace InstalledOpenLimitTilt
namespace InstalledClosedLimitTilt {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = InstalledClosedLimitTilt::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace InstalledClosedLimitTilt
namespace Mode {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Mode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace Mode
namespace SafetyStatus {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SafetyStatus::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SafetyStatus

} // namespace Attributes

namespace Commands {
namespace UpOrOpen {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = UpOrOpen::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace UpOrOpen
namespace DownOrClose {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = DownOrClose::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace DownOrClose
namespace StopMotion {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = StopMotion::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace StopMotion
namespace GoToLiftValue {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = GoToLiftValue::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace GoToLiftValue
namespace GoToLiftPercentage {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = GoToLiftPercentage::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace GoToLiftPercentage
namespace GoToTiltValue {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = GoToTiltValue::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace GoToTiltValue
namespace GoToTiltPercentage {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = GoToTiltPercentage::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace GoToTiltPercentage

} // namespace Commands
} // namespace WindowCovering
} // namespace Clusters
} // namespace app
} // namespace chip
