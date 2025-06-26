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
inline constexpr DataModel::AttributeEntry kMetadataEntry(Type::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Type
namespace PhysicalClosedLimitLift {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PhysicalClosedLimitLift::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PhysicalClosedLimitLift
namespace PhysicalClosedLimitTilt {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PhysicalClosedLimitTilt::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PhysicalClosedLimitTilt
namespace CurrentPositionLift {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentPositionLift::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentPositionLift
namespace CurrentPositionTilt {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentPositionTilt::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentPositionTilt
namespace NumberOfActuationsLift {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NumberOfActuationsLift::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace NumberOfActuationsLift
namespace NumberOfActuationsTilt {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NumberOfActuationsTilt::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace NumberOfActuationsTilt
namespace ConfigStatus {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ConfigStatus::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ConfigStatus
namespace CurrentPositionLiftPercentage {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentPositionLiftPercentage::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace CurrentPositionLiftPercentage
namespace CurrentPositionTiltPercentage {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentPositionTiltPercentage::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace CurrentPositionTiltPercentage
namespace OperationalStatus {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OperationalStatus::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace OperationalStatus
namespace TargetPositionLiftPercent100ths {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TargetPositionLiftPercent100ths::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace TargetPositionLiftPercent100ths
namespace TargetPositionTiltPercent100ths {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TargetPositionTiltPercent100ths::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace TargetPositionTiltPercent100ths
namespace EndProductType {
inline constexpr DataModel::AttributeEntry kMetadataEntry(EndProductType::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace EndProductType
namespace CurrentPositionLiftPercent100ths {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentPositionLiftPercent100ths::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace CurrentPositionLiftPercent100ths
namespace CurrentPositionTiltPercent100ths {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentPositionTiltPercent100ths::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace CurrentPositionTiltPercent100ths
namespace InstalledOpenLimitLift {
inline constexpr DataModel::AttributeEntry kMetadataEntry(InstalledOpenLimitLift::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace InstalledOpenLimitLift
namespace InstalledClosedLimitLift {
inline constexpr DataModel::AttributeEntry kMetadataEntry(InstalledClosedLimitLift::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace InstalledClosedLimitLift
namespace InstalledOpenLimitTilt {
inline constexpr DataModel::AttributeEntry kMetadataEntry(InstalledOpenLimitTilt::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace InstalledOpenLimitTilt
namespace InstalledClosedLimitTilt {
inline constexpr DataModel::AttributeEntry kMetadataEntry(InstalledClosedLimitTilt::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace InstalledClosedLimitTilt
namespace Mode {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Mode::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace Mode
namespace SafetyStatus {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SafetyStatus::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SafetyStatus

} // namespace Attributes

namespace Commands {
namespace UpOrOpen {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(UpOrOpen::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace UpOrOpen
namespace DownOrClose {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(DownOrClose::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace DownOrClose
namespace StopMotion {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(StopMotion::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace StopMotion
namespace GoToLiftValue {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(GoToLiftValue::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace GoToLiftValue
namespace GoToLiftPercentage {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(GoToLiftPercentage::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace GoToLiftPercentage
namespace GoToTiltValue {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(GoToTiltValue::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace GoToTiltValue
namespace GoToTiltPercentage {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(GoToTiltPercentage::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace GoToTiltPercentage

} // namespace Commands
} // namespace WindowCovering
} // namespace Clusters
} // namespace app
} // namespace chip
