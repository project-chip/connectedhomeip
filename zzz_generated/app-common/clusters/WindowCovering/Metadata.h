// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster WindowCovering (cluster code: 258/0x102)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/WindowCovering/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WindowCovering {

inline constexpr uint32_t kRevision = 8;

namespace Attributes {

namespace Type {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Type::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Type
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
namespace Mode {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Mode::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace Mode
namespace SafetyStatus {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SafetyStatus::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SafetyStatus
constexpr std::array<DataModel::AttributeEntry, 5> kMandatoryMetadata = {
    Type::kMetadataEntry,           ConfigStatus::kMetadataEntry, OperationalStatus::kMetadataEntry,
    EndProductType::kMetadataEntry, Mode::kMetadataEntry,

};

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
namespace GoToLiftPercentage {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(GoToLiftPercentage::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace GoToLiftPercentage
namespace GoToTiltPercentage {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(GoToTiltPercentage::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace GoToTiltPercentage

} // namespace Commands

namespace Events {} // namespace Events
} // namespace WindowCovering
} // namespace Clusters
} // namespace app
} // namespace chip
