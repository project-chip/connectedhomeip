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
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentHue::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentHue
namespace CurrentSaturation {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentSaturation::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentSaturation
namespace RemainingTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RemainingTime::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace RemainingTime
namespace CurrentX {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentX::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentX
namespace CurrentY {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentY::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentY
namespace DriftCompensation {
inline constexpr DataModel::AttributeEntry kMetadataEntry(DriftCompensation::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace DriftCompensation
namespace CompensationText {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CompensationText::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CompensationText
namespace ColorTemperatureMireds {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ColorTemperatureMireds::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ColorTemperatureMireds
namespace ColorMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ColorMode::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ColorMode
namespace Options {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Options::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Options
namespace NumberOfPrimaries {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NumberOfPrimaries::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace NumberOfPrimaries
namespace Primary1X {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Primary1X::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Primary1X
namespace Primary1Y {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Primary1Y::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Primary1Y
namespace Primary1Intensity {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Primary1Intensity::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Primary1Intensity
namespace Primary2X {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Primary2X::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Primary2X
namespace Primary2Y {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Primary2Y::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Primary2Y
namespace Primary2Intensity {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Primary2Intensity::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Primary2Intensity
namespace Primary3X {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Primary3X::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Primary3X
namespace Primary3Y {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Primary3Y::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Primary3Y
namespace Primary3Intensity {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Primary3Intensity::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Primary3Intensity
namespace Primary4X {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Primary4X::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Primary4X
namespace Primary4Y {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Primary4Y::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Primary4Y
namespace Primary4Intensity {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Primary4Intensity::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Primary4Intensity
namespace Primary5X {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Primary5X::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Primary5X
namespace Primary5Y {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Primary5Y::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Primary5Y
namespace Primary5Intensity {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Primary5Intensity::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Primary5Intensity
namespace Primary6X {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Primary6X::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Primary6X
namespace Primary6Y {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Primary6Y::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Primary6Y
namespace Primary6Intensity {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Primary6Intensity::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Primary6Intensity
namespace WhitePointX {
inline constexpr DataModel::AttributeEntry kMetadataEntry(WhitePointX::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace WhitePointX
namespace WhitePointY {
inline constexpr DataModel::AttributeEntry kMetadataEntry(WhitePointY::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace WhitePointY
namespace ColorPointRX {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ColorPointRX::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace ColorPointRX
namespace ColorPointRY {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ColorPointRY::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace ColorPointRY
namespace ColorPointRIntensity {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ColorPointRIntensity::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace ColorPointRIntensity
namespace ColorPointGX {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ColorPointGX::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace ColorPointGX
namespace ColorPointGY {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ColorPointGY::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace ColorPointGY
namespace ColorPointGIntensity {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ColorPointGIntensity::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace ColorPointGIntensity
namespace ColorPointBX {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ColorPointBX::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace ColorPointBX
namespace ColorPointBY {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ColorPointBY::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace ColorPointBY
namespace ColorPointBIntensity {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ColorPointBIntensity::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace ColorPointBIntensity
namespace EnhancedCurrentHue {
inline constexpr DataModel::AttributeEntry kMetadataEntry(EnhancedCurrentHue::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace EnhancedCurrentHue
namespace EnhancedColorMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry(EnhancedColorMode::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace EnhancedColorMode
namespace ColorLoopActive {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ColorLoopActive::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ColorLoopActive
namespace ColorLoopDirection {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ColorLoopDirection::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ColorLoopDirection
namespace ColorLoopTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ColorLoopTime::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ColorLoopTime
namespace ColorLoopStartEnhancedHue {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ColorLoopStartEnhancedHue::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace ColorLoopStartEnhancedHue
namespace ColorLoopStoredEnhancedHue {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ColorLoopStoredEnhancedHue::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace ColorLoopStoredEnhancedHue
namespace ColorCapabilities {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ColorCapabilities::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ColorCapabilities
namespace ColorTempPhysicalMinMireds {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ColorTempPhysicalMinMireds::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace ColorTempPhysicalMinMireds
namespace ColorTempPhysicalMaxMireds {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ColorTempPhysicalMaxMireds::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace ColorTempPhysicalMaxMireds
namespace CoupleColorTempToLevelMinMireds {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CoupleColorTempToLevelMinMireds::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace CoupleColorTempToLevelMinMireds
namespace StartUpColorTemperatureMireds {
inline constexpr DataModel::AttributeEntry kMetadataEntry(StartUpColorTemperatureMireds::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kManage);
} // namespace StartUpColorTemperatureMireds

} // namespace Attributes

namespace Commands {
namespace MoveToHue {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(MoveToHue::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace MoveToHue
namespace MoveHue {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(MoveHue::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace MoveHue
namespace StepHue {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(StepHue::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace StepHue
namespace MoveToSaturation {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(MoveToSaturation::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace MoveToSaturation
namespace MoveSaturation {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(MoveSaturation::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace MoveSaturation
namespace StepSaturation {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(StepSaturation::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace StepSaturation
namespace MoveToHueAndSaturation {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(MoveToHueAndSaturation::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace MoveToHueAndSaturation
namespace MoveToColor {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(MoveToColor::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace MoveToColor
namespace MoveColor {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(MoveColor::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace MoveColor
namespace StepColor {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(StepColor::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace StepColor
namespace MoveToColorTemperature {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(MoveToColorTemperature::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace MoveToColorTemperature
namespace EnhancedMoveToHue {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(EnhancedMoveToHue::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace EnhancedMoveToHue
namespace EnhancedMoveHue {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(EnhancedMoveHue::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace EnhancedMoveHue
namespace EnhancedStepHue {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(EnhancedStepHue::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace EnhancedStepHue
namespace EnhancedMoveToHueAndSaturation {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(EnhancedMoveToHueAndSaturation::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace EnhancedMoveToHueAndSaturation
namespace ColorLoopSet {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ColorLoopSet::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace ColorLoopSet
namespace StopMoveStep {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(StopMoveStep::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace StopMoveStep
namespace MoveColorTemperature {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(MoveColorTemperature::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace MoveColorTemperature
namespace StepColorTemperature {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(StepColorTemperature::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace StepColorTemperature

} // namespace Commands
} // namespace ColorControl
} // namespace Clusters
} // namespace app
} // namespace chip
