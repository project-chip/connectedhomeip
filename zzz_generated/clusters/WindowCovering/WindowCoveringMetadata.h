// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster WindowCovering (cluster code: 258/0x102)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/WindowCovering/WindowCoveringIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace WindowCovering {
namespace Metadata {

inline constexpr uint32_t kRevision = 5;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kTypeEntry = {
    .attributeId    = WindowCovering::Attributes::Type::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPhysicalClosedLimitLiftEntry = {
    .attributeId    = WindowCovering::Attributes::PhysicalClosedLimitLift::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPhysicalClosedLimitTiltEntry = {
    .attributeId    = WindowCovering::Attributes::PhysicalClosedLimitTilt::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCurrentPositionLiftEntry = {
    .attributeId    = WindowCovering::Attributes::CurrentPositionLift::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCurrentPositionTiltEntry = {
    .attributeId    = WindowCovering::Attributes::CurrentPositionTilt::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNumberOfActuationsLiftEntry = {
    .attributeId    = WindowCovering::Attributes::NumberOfActuationsLift::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNumberOfActuationsTiltEntry = {
    .attributeId    = WindowCovering::Attributes::NumberOfActuationsTilt::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kConfigStatusEntry = {
    .attributeId    = WindowCovering::Attributes::ConfigStatus::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCurrentPositionLiftPercentageEntry = {
    .attributeId    = WindowCovering::Attributes::CurrentPositionLiftPercentage::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCurrentPositionTiltPercentageEntry = {
    .attributeId    = WindowCovering::Attributes::CurrentPositionTiltPercentage::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kOperationalStatusEntry = {
    .attributeId    = WindowCovering::Attributes::OperationalStatus::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTargetPositionLiftPercent100thsEntry = {
    .attributeId    = WindowCovering::Attributes::TargetPositionLiftPercent100ths::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTargetPositionTiltPercent100thsEntry = {
    .attributeId    = WindowCovering::Attributes::TargetPositionTiltPercent100ths::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kEndProductTypeEntry = {
    .attributeId    = WindowCovering::Attributes::EndProductType::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCurrentPositionLiftPercent100thsEntry = {
    .attributeId    = WindowCovering::Attributes::CurrentPositionLiftPercent100ths::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCurrentPositionTiltPercent100thsEntry = {
    .attributeId    = WindowCovering::Attributes::CurrentPositionTiltPercent100ths::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kInstalledOpenLimitLiftEntry = {
    .attributeId    = WindowCovering::Attributes::InstalledOpenLimitLift::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kInstalledClosedLimitLiftEntry = {
    .attributeId    = WindowCovering::Attributes::InstalledClosedLimitLift::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kInstalledOpenLimitTiltEntry = {
    .attributeId    = WindowCovering::Attributes::InstalledOpenLimitTilt::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kInstalledClosedLimitTiltEntry = {
    .attributeId    = WindowCovering::Attributes::InstalledClosedLimitTilt::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kModeEntry = {
    .attributeId    = WindowCovering::Attributes::Mode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kSafetyStatusEntry = {
    .attributeId    = WindowCovering::Attributes::SafetyStatus::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kUpOrOpenEntry = {
    .commandId       = WindowCovering::Commands::UpOrOpen::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kDownOrCloseEntry = {
    .commandId       = WindowCovering::Commands::DownOrClose::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kStopMotionEntry = {
    .commandId       = WindowCovering::Commands::StopMotion::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kGoToLiftValueEntry = {
    .commandId       = WindowCovering::Commands::GoToLiftValue::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kGoToLiftPercentageEntry = {
    .commandId       = WindowCovering::Commands::GoToLiftPercentage::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kGoToTiltValueEntry = {
    .commandId       = WindowCovering::Commands::GoToTiltValue::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kGoToTiltPercentageEntry = {
    .commandId       = WindowCovering::Commands::GoToTiltPercentage::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace WindowCovering
} // namespace clusters
} // namespace app
} // namespace chip
