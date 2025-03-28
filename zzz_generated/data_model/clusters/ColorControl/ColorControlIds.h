// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ColorControl (cluster code: 768/0x300)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ColorControl {

inline constexpr ClusterId Id = 0x00000300;

namespace Attributes {
namespace CurrentHue {
inline constexpr AttributeId Id = 0x00000000;
} // namespace CurrentHue
namespace CurrentSaturation {
inline constexpr AttributeId Id = 0x00000001;
} // namespace CurrentSaturation
namespace RemainingTime {
inline constexpr AttributeId Id = 0x00000002;
} // namespace RemainingTime
namespace CurrentX {
inline constexpr AttributeId Id = 0x00000003;
} // namespace CurrentX
namespace CurrentY {
inline constexpr AttributeId Id = 0x00000004;
} // namespace CurrentY
namespace DriftCompensation {
inline constexpr AttributeId Id = 0x00000005;
} // namespace DriftCompensation
namespace CompensationText {
inline constexpr AttributeId Id = 0x00000006;
} // namespace CompensationText
namespace ColorTemperatureMireds {
inline constexpr AttributeId Id = 0x00000007;
} // namespace ColorTemperatureMireds
namespace ColorMode {
inline constexpr AttributeId Id = 0x00000008;
} // namespace ColorMode
namespace Options {
inline constexpr AttributeId Id = 0x0000000F;
} // namespace Options
namespace NumberOfPrimaries {
inline constexpr AttributeId Id = 0x00000010;
} // namespace NumberOfPrimaries
namespace Primary1X {
inline constexpr AttributeId Id = 0x00000011;
} // namespace Primary1X
namespace Primary1Y {
inline constexpr AttributeId Id = 0x00000012;
} // namespace Primary1Y
namespace Primary1Intensity {
inline constexpr AttributeId Id = 0x00000013;
} // namespace Primary1Intensity
namespace Primary2X {
inline constexpr AttributeId Id = 0x00000015;
} // namespace Primary2X
namespace Primary2Y {
inline constexpr AttributeId Id = 0x00000016;
} // namespace Primary2Y
namespace Primary2Intensity {
inline constexpr AttributeId Id = 0x00000017;
} // namespace Primary2Intensity
namespace Primary3X {
inline constexpr AttributeId Id = 0x00000019;
} // namespace Primary3X
namespace Primary3Y {
inline constexpr AttributeId Id = 0x0000001A;
} // namespace Primary3Y
namespace Primary3Intensity {
inline constexpr AttributeId Id = 0x0000001B;
} // namespace Primary3Intensity
namespace Primary4X {
inline constexpr AttributeId Id = 0x00000020;
} // namespace Primary4X
namespace Primary4Y {
inline constexpr AttributeId Id = 0x00000021;
} // namespace Primary4Y
namespace Primary4Intensity {
inline constexpr AttributeId Id = 0x00000022;
} // namespace Primary4Intensity
namespace Primary5X {
inline constexpr AttributeId Id = 0x00000024;
} // namespace Primary5X
namespace Primary5Y {
inline constexpr AttributeId Id = 0x00000025;
} // namespace Primary5Y
namespace Primary5Intensity {
inline constexpr AttributeId Id = 0x00000026;
} // namespace Primary5Intensity
namespace Primary6X {
inline constexpr AttributeId Id = 0x00000028;
} // namespace Primary6X
namespace Primary6Y {
inline constexpr AttributeId Id = 0x00000029;
} // namespace Primary6Y
namespace Primary6Intensity {
inline constexpr AttributeId Id = 0x0000002A;
} // namespace Primary6Intensity
namespace WhitePointX {
inline constexpr AttributeId Id = 0x00000030;
} // namespace WhitePointX
namespace WhitePointY {
inline constexpr AttributeId Id = 0x00000031;
} // namespace WhitePointY
namespace ColorPointRX {
inline constexpr AttributeId Id = 0x00000032;
} // namespace ColorPointRX
namespace ColorPointRY {
inline constexpr AttributeId Id = 0x00000033;
} // namespace ColorPointRY
namespace ColorPointRIntensity {
inline constexpr AttributeId Id = 0x00000034;
} // namespace ColorPointRIntensity
namespace ColorPointGX {
inline constexpr AttributeId Id = 0x00000036;
} // namespace ColorPointGX
namespace ColorPointGY {
inline constexpr AttributeId Id = 0x00000037;
} // namespace ColorPointGY
namespace ColorPointGIntensity {
inline constexpr AttributeId Id = 0x00000038;
} // namespace ColorPointGIntensity
namespace ColorPointBX {
inline constexpr AttributeId Id = 0x0000003A;
} // namespace ColorPointBX
namespace ColorPointBY {
inline constexpr AttributeId Id = 0x0000003B;
} // namespace ColorPointBY
namespace ColorPointBIntensity {
inline constexpr AttributeId Id = 0x0000003C;
} // namespace ColorPointBIntensity
namespace EnhancedCurrentHue {
inline constexpr AttributeId Id = 0x00004000;
} // namespace EnhancedCurrentHue
namespace EnhancedColorMode {
inline constexpr AttributeId Id = 0x00004001;
} // namespace EnhancedColorMode
namespace ColorLoopActive {
inline constexpr AttributeId Id = 0x00004002;
} // namespace ColorLoopActive
namespace ColorLoopDirection {
inline constexpr AttributeId Id = 0x00004003;
} // namespace ColorLoopDirection
namespace ColorLoopTime {
inline constexpr AttributeId Id = 0x00004004;
} // namespace ColorLoopTime
namespace ColorLoopStartEnhancedHue {
inline constexpr AttributeId Id = 0x00004005;
} // namespace ColorLoopStartEnhancedHue
namespace ColorLoopStoredEnhancedHue {
inline constexpr AttributeId Id = 0x00004006;
} // namespace ColorLoopStoredEnhancedHue
namespace ColorCapabilities {
inline constexpr AttributeId Id = 0x0000400A;
} // namespace ColorCapabilities
namespace ColorTempPhysicalMinMireds {
inline constexpr AttributeId Id = 0x0000400B;
} // namespace ColorTempPhysicalMinMireds
namespace ColorTempPhysicalMaxMireds {
inline constexpr AttributeId Id = 0x0000400C;
} // namespace ColorTempPhysicalMaxMireds
namespace CoupleColorTempToLevelMinMireds {
inline constexpr AttributeId Id = 0x0000400D;
} // namespace CoupleColorTempToLevelMinMireds
namespace StartUpColorTemperatureMireds {
inline constexpr AttributeId Id = 0x00004010;
} // namespace StartUpColorTemperatureMireds
namespace GeneratedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::GeneratedCommandList::Id;
} // namespace GeneratedCommandList
namespace AcceptedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::AcceptedCommandList::Id;
} // namespace AcceptedCommandList
namespace EventList {
inline constexpr AttributeId Id = Globals::Attributes::EventList::Id;
} // namespace EventList
namespace AttributeList {
inline constexpr AttributeId Id = Globals::Attributes::AttributeList::Id;
} // namespace AttributeList
namespace FeatureMap {
inline constexpr AttributeId Id = Globals::Attributes::FeatureMap::Id;
} // namespace FeatureMap
namespace ClusterRevision {
inline constexpr AttributeId Id = Globals::Attributes::ClusterRevision::Id;
} // namespace ClusterRevision

} // namespace Attributes

namespace Commands {
namespace MoveToHue {
inline constexpr CommandId Id = 0x00000000;
} // namespace MoveToHue
namespace MoveHue {
inline constexpr CommandId Id = 0x00000001;
} // namespace MoveHue
namespace StepHue {
inline constexpr CommandId Id = 0x00000002;
} // namespace StepHue
namespace MoveToSaturation {
inline constexpr CommandId Id = 0x00000003;
} // namespace MoveToSaturation
namespace MoveSaturation {
inline constexpr CommandId Id = 0x00000004;
} // namespace MoveSaturation
namespace StepSaturation {
inline constexpr CommandId Id = 0x00000005;
} // namespace StepSaturation
namespace MoveToHueAndSaturation {
inline constexpr CommandId Id = 0x00000006;
} // namespace MoveToHueAndSaturation
namespace MoveToColor {
inline constexpr CommandId Id = 0x00000007;
} // namespace MoveToColor
namespace MoveColor {
inline constexpr CommandId Id = 0x00000008;
} // namespace MoveColor
namespace StepColor {
inline constexpr CommandId Id = 0x00000009;
} // namespace StepColor
namespace MoveToColorTemperature {
inline constexpr CommandId Id = 0x0000000A;
} // namespace MoveToColorTemperature
namespace EnhancedMoveToHue {
inline constexpr CommandId Id = 0x00000040;
} // namespace EnhancedMoveToHue
namespace EnhancedMoveHue {
inline constexpr CommandId Id = 0x00000041;
} // namespace EnhancedMoveHue
namespace EnhancedStepHue {
inline constexpr CommandId Id = 0x00000042;
} // namespace EnhancedStepHue
namespace EnhancedMoveToHueAndSaturation {
inline constexpr CommandId Id = 0x00000043;
} // namespace EnhancedMoveToHueAndSaturation
namespace ColorLoopSet {
inline constexpr CommandId Id = 0x00000044;
} // namespace ColorLoopSet
namespace StopMoveStep {
inline constexpr CommandId Id = 0x00000047;
} // namespace StopMoveStep
namespace MoveColorTemperature {
inline constexpr CommandId Id = 0x0000004B;
} // namespace MoveColorTemperature
namespace StepColorTemperature {
inline constexpr CommandId Id = 0x0000004C;
} // namespace StepColorTemperature
} // namespace Commands

namespace Events {} // namespace Events

} // namespace ColorControl
} // namespace Clusters
} // namespace app
} // namespace chip
