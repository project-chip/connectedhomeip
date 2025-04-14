// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ColorControl (cluster code: 768/0x300)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ColorControl/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ColorControl {

inline constexpr uint32_t kRevision = 7;

namespace Attributes {
namespace CurrentHue {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::CurrentHue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentHue
namespace CurrentSaturation {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::CurrentSaturation::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentSaturation
namespace RemainingTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::RemainingTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RemainingTime
namespace CurrentX {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::CurrentX::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentX
namespace CurrentY {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::CurrentY::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentY
namespace DriftCompensation {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::DriftCompensation::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace DriftCompensation
namespace CompensationText {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::CompensationText::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CompensationText
namespace ColorTemperatureMireds {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::ColorTemperatureMireds::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ColorTemperatureMireds
namespace ColorMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::ColorMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ColorMode
namespace Options {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::Options::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace Options
namespace NumberOfPrimaries {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::NumberOfPrimaries::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NumberOfPrimaries
namespace Primary1X {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::Primary1X::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Primary1X
namespace Primary1Y {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::Primary1Y::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Primary1Y
namespace Primary1Intensity {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::Primary1Intensity::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Primary1Intensity
namespace Primary2X {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::Primary2X::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Primary2X
namespace Primary2Y {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::Primary2Y::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Primary2Y
namespace Primary2Intensity {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::Primary2Intensity::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Primary2Intensity
namespace Primary3X {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::Primary3X::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Primary3X
namespace Primary3Y {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::Primary3Y::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Primary3Y
namespace Primary3Intensity {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::Primary3Intensity::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Primary3Intensity
namespace Primary4X {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::Primary4X::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Primary4X
namespace Primary4Y {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::Primary4Y::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Primary4Y
namespace Primary4Intensity {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::Primary4Intensity::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Primary4Intensity
namespace Primary5X {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::Primary5X::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Primary5X
namespace Primary5Y {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::Primary5Y::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Primary5Y
namespace Primary5Intensity {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::Primary5Intensity::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Primary5Intensity
namespace Primary6X {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::Primary6X::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Primary6X
namespace Primary6Y {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::Primary6Y::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Primary6Y
namespace Primary6Intensity {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::Primary6Intensity::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Primary6Intensity
namespace WhitePointX {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::WhitePointX::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace WhitePointX
namespace WhitePointY {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::WhitePointY::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace WhitePointY
namespace ColorPointRX {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::ColorPointRX::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace ColorPointRX
namespace ColorPointRY {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::ColorPointRY::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace ColorPointRY
namespace ColorPointRIntensity {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::ColorPointRIntensity::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace ColorPointRIntensity
namespace ColorPointGX {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::ColorPointGX::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace ColorPointGX
namespace ColorPointGY {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::ColorPointGY::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace ColorPointGY
namespace ColorPointGIntensity {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::ColorPointGIntensity::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace ColorPointGIntensity
namespace ColorPointBX {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::ColorPointBX::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace ColorPointBX
namespace ColorPointBY {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::ColorPointBY::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace ColorPointBY
namespace ColorPointBIntensity {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::ColorPointBIntensity::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace ColorPointBIntensity
namespace EnhancedCurrentHue {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::EnhancedCurrentHue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace EnhancedCurrentHue
namespace EnhancedColorMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::EnhancedColorMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace EnhancedColorMode
namespace ColorLoopActive {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::ColorLoopActive::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ColorLoopActive
namespace ColorLoopDirection {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::ColorLoopDirection::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ColorLoopDirection
namespace ColorLoopTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::ColorLoopTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ColorLoopTime
namespace ColorLoopStartEnhancedHue {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::ColorLoopStartEnhancedHue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ColorLoopStartEnhancedHue
namespace ColorLoopStoredEnhancedHue {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::ColorLoopStoredEnhancedHue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ColorLoopStoredEnhancedHue
namespace ColorCapabilities {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::ColorCapabilities::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ColorCapabilities
namespace ColorTempPhysicalMinMireds {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::ColorTempPhysicalMinMireds::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ColorTempPhysicalMinMireds
namespace ColorTempPhysicalMaxMireds {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::ColorTempPhysicalMaxMireds::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ColorTempPhysicalMaxMireds
namespace CoupleColorTempToLevelMinMireds {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::CoupleColorTempToLevelMinMireds::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CoupleColorTempToLevelMinMireds
namespace StartUpColorTemperatureMireds {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ColorControl::Attributes::StartUpColorTemperatureMireds::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace StartUpColorTemperatureMireds

} // namespace Attributes

namespace Commands {
namespace MoveToHue {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ColorControl::Commands::MoveToHue::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace MoveToHue
namespace MoveHue {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ColorControl::Commands::MoveHue::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace MoveHue
namespace StepHue {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ColorControl::Commands::StepHue::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace StepHue
namespace MoveToSaturation {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ColorControl::Commands::MoveToSaturation::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace MoveToSaturation
namespace MoveSaturation {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ColorControl::Commands::MoveSaturation::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace MoveSaturation
namespace StepSaturation {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ColorControl::Commands::StepSaturation::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace StepSaturation
namespace MoveToHueAndSaturation {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ColorControl::Commands::MoveToHueAndSaturation::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace MoveToHueAndSaturation
namespace MoveToColor {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ColorControl::Commands::MoveToColor::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace MoveToColor
namespace MoveColor {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ColorControl::Commands::MoveColor::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace MoveColor
namespace StepColor {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ColorControl::Commands::StepColor::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace StepColor
namespace MoveToColorTemperature {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ColorControl::Commands::MoveToColorTemperature::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace MoveToColorTemperature
namespace EnhancedMoveToHue {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ColorControl::Commands::EnhancedMoveToHue::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace EnhancedMoveToHue
namespace EnhancedMoveHue {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ColorControl::Commands::EnhancedMoveHue::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace EnhancedMoveHue
namespace EnhancedStepHue {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ColorControl::Commands::EnhancedStepHue::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace EnhancedStepHue
namespace EnhancedMoveToHueAndSaturation {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ColorControl::Commands::EnhancedMoveToHueAndSaturation::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace EnhancedMoveToHueAndSaturation
namespace ColorLoopSet {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ColorControl::Commands::ColorLoopSet::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace ColorLoopSet
namespace StopMoveStep {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ColorControl::Commands::StopMoveStep::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace StopMoveStep
namespace MoveColorTemperature {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ColorControl::Commands::MoveColorTemperature::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace MoveColorTemperature
namespace StepColorTemperature {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ColorControl::Commands::StepColorTemperature::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace StepColorTemperature

} // namespace Commands
} // namespace ColorControl
} // namespace Clusters
} // namespace app
} // namespace chip
