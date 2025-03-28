// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster WindowCovering (cluster code: 258/0x102)
// based on src/controller/data_model/controller-clusters.matter

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
    .attributeId    = Attributes::Type::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPhysicalClosedLimitLiftEntry = {
    .attributeId    = Attributes::PhysicalClosedLimitLift::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPhysicalClosedLimitTiltEntry = {
    .attributeId    = Attributes::PhysicalClosedLimitTilt::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCurrentPositionLiftEntry = {
    .attributeId    = Attributes::CurrentPositionLift::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCurrentPositionTiltEntry = {
    .attributeId    = Attributes::CurrentPositionTilt::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNumberOfActuationsLiftEntry = {
    .attributeId    = Attributes::NumberOfActuationsLift::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNumberOfActuationsTiltEntry = {
    .attributeId    = Attributes::NumberOfActuationsTilt::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kConfigStatusEntry = {
    .attributeId    = Attributes::ConfigStatus::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCurrentPositionLiftPercentageEntry = {
    .attributeId    = Attributes::CurrentPositionLiftPercentage::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCurrentPositionTiltPercentageEntry = {
    .attributeId    = Attributes::CurrentPositionTiltPercentage::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kOperationalStatusEntry = {
    .attributeId    = Attributes::OperationalStatus::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTargetPositionLiftPercent100thsEntry = {
    .attributeId    = Attributes::TargetPositionLiftPercent100ths::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTargetPositionTiltPercent100thsEntry = {
    .attributeId    = Attributes::TargetPositionTiltPercent100ths::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kEndProductTypeEntry = {
    .attributeId    = Attributes::EndProductType::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCurrentPositionLiftPercent100thsEntry = {
    .attributeId    = Attributes::CurrentPositionLiftPercent100ths::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCurrentPositionTiltPercent100thsEntry = {
    .attributeId    = Attributes::CurrentPositionTiltPercent100ths::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kInstalledOpenLimitLiftEntry = {
    .attributeId    = Attributes::InstalledOpenLimitLift::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kInstalledClosedLimitLiftEntry = {
    .attributeId    = Attributes::InstalledClosedLimitLift::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kInstalledOpenLimitTiltEntry = {
    .attributeId    = Attributes::InstalledOpenLimitTilt::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kInstalledClosedLimitTiltEntry = {
    .attributeId    = Attributes::InstalledClosedLimitTilt::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kModeEntry = {
    .attributeId    = Attributes::Mode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kSafetyStatusEntry = {
    .attributeId    = Attributes::SafetyStatus::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kUpOrOpenEntry = {
    .commandId       = Commands::UpOrOpen::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kDownOrCloseEntry = {
    .commandId       = Commands::DownOrClose::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kStopMotionEntry = {
    .commandId       = Commands::StopMotion::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kGoToLiftValueEntry = {
    .commandId       = Commands::GoToLiftValue::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kGoToLiftPercentageEntry = {
    .commandId       = Commands::GoToLiftPercentage::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kGoToTiltValueEntry = {
    .commandId       = Commands::GoToTiltValue::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kGoToTiltPercentageEntry = {
    .commandId       = Commands::GoToTiltPercentage::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace WindowCovering
} // namespace clusters
} // namespace app
} // namespace chip
