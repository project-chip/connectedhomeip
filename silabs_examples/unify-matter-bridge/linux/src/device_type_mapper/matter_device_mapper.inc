/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

// This file is generated automatically. Don't try to change something here.
// To add support for new clusters, modify addon-helper.js in uic/applications/matter_bridge/src/zap

// Component include
#include <matter_device_translator.hpp>

// Standard library 
#include <string>
#include <optional>
#include <unordered_map>

// Matter includes
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Commands.h>

// Cluster mapping Unify->Matter conversion
namespace chip {
namespace app {
namespace Clusters {

static const std::unordered_map<std::string, chip::ClusterId> unify_cluster_id_map {
    { "Basic", BridgedDeviceBasic::Id},
    { "Identify", Identify::Id},
    { "Groups", Groups::Id},
    { "Scenes", Scenes::Id},
    { "OnOff", OnOff::Id},
    { "Level", LevelControl::Id},
    { "FanControl", FanControl::Id},
    { "ColorControl", ColorControl::Id},
    { "IlluminanceMeasurement", IlluminanceMeasurement::Id},
    { "TemperatureMeasurement", TemperatureMeasurement::Id},
    { "PressureMeasurement", PressureMeasurement::Id},
    { "FlowMeasurement", FlowMeasurement::Id},
    { "OccupancySensing", OccupancySensing::Id},
};

// Attribute mapping Unify->Matter conversion
// Attribute structure for each cluster
namespace BridgedDeviceBasic {
namespace Attributes {
static const std::unordered_map<std::string, chip::AttributeId> unify_to_matter_attribute_id_cluster_map {
    { "ApplicationVersion", SoftwareVersion::Id },
    { "HwVersion", HardwareVersion::Id },
    { "ManufacturerName", VendorName::Id },
    { "ModelIdentifier", ProductName::Id },
    { "DateCode", ManufacturingDate::Id },
    { "PowerSource", PowerSource::Id },
    { "ProductUrL", ProductURL::Id },
    { "SerialNumber", SerialNumber::Id },
    { "ProductLabel", ProductLabel::Id },
    { "SwBuildID", SoftwareVersionString::Id }
};
} // namespace Attributes
} // namespace Basic

// Attribute mapping Unify->Matter conversion
// Attribute structure for each cluster
namespace Identify {
namespace Attributes {
static const std::unordered_map<std::string, chip::AttributeId> unify_to_matter_attribute_id_cluster_map {
    { "IdentifyTime", IdentifyTime::Id }
};
} // namespace Attributes
} // namespace Identify
namespace Groups {
namespace Attributes {
static const std::unordered_map<std::string, chip::AttributeId> unify_to_matter_attribute_id_cluster_map {
    { "NameSupport", NameSupport::Id }
};
} // namespace Attributes
} // namespace Groups
namespace Scenes {
namespace Attributes {
static const std::unordered_map<std::string, chip::AttributeId> unify_to_matter_attribute_id_cluster_map {
    { "SceneCount", SceneCount::Id },
    { "CurrentScene", CurrentScene::Id },
    { "CurrentGroup", CurrentGroup::Id },
    { "SceneValid", SceneValid::Id },
    { "NameSupport", NameSupport::Id },
    { "LastConfiguredBy", LastConfiguredBy::Id }
};
} // namespace Attributes
} // namespace Scenes
namespace OnOff {
namespace Attributes {
static const std::unordered_map<std::string, chip::AttributeId> unify_to_matter_attribute_id_cluster_map {
    { "OnOff", OnOff::Id },
    { "GlobalSceneControl", GlobalSceneControl::Id },
    { "OnTime", OnTime::Id },
    { "OffWaitTime", OffWaitTime::Id },
    { "StartUpOnOff", StartUpOnOff::Id }
};
} // namespace Attributes
} // namespace OnOff
namespace LevelControl {
namespace Attributes {
static const std::unordered_map<std::string, chip::AttributeId> unify_to_matter_attribute_id_cluster_map {
    { "CurrentLevel", CurrentLevel::Id },
    { "RemainingTime", RemainingTime::Id },
    { "MinLevel", MinLevel::Id },
    { "MaxLevel", MaxLevel::Id },
    { "CurrentFrequency", CurrentFrequency::Id },
    { "MinFrequency", MinFrequency::Id },
    { "MaxFrequency", MaxFrequency::Id },
    { "Options", Options::Id },
    { "OnOffTransitionTime", OnOffTransitionTime::Id },
    { "OnLevel", OnLevel::Id },
    { "OnTransitionTime", OnTransitionTime::Id },
    { "OffTransitionTime", OffTransitionTime::Id },
    { "DefaultMoveRate", DefaultMoveRate::Id },
    { "StartUpCurrentLevel", StartUpCurrentLevel::Id }
};
} // namespace Attributes
} // namespace Level
namespace FanControl {
namespace Attributes {
static const std::unordered_map<std::string, chip::AttributeId> unify_to_matter_attribute_id_cluster_map {
    { "FanMode", FanMode::Id },
    { "FanModeSequence", FanModeSequence::Id }
};
} // namespace Attributes
} // namespace FanControl
namespace ColorControl {
namespace Attributes {
static const std::unordered_map<std::string, chip::AttributeId> unify_to_matter_attribute_id_cluster_map {
    { "CurrentHue", CurrentHue::Id },
    { "CurrentSaturation", CurrentSaturation::Id },
    { "RemainingTime", RemainingTime::Id },
    { "CurrentX", CurrentX::Id },
    { "CurrentY", CurrentY::Id },
    { "DriftCompensation", DriftCompensation::Id },
    { "CompensationText", CompensationText::Id },
    { "ColorTemperatureMireds", ColorTemperatureMireds::Id },
    { "ColorMode", ColorMode::Id },
    { "NumberOfPrimaries", NumberOfPrimaries::Id },
    { "Primary1X", Primary1X::Id },
    { "Primary1Y", Primary1Y::Id },
    { "Primary1Intensity", Primary1Intensity::Id },
    { "Primary2X", Primary2X::Id },
    { "Primary2Y", Primary2Y::Id },
    { "Primary2Intensity", Primary2Intensity::Id },
    { "Primary3X", Primary3X::Id },
    { "Primary3Y", Primary3Y::Id },
    { "Primary3Intensity", Primary3Intensity::Id },
    { "Primary4X", Primary4X::Id },
    { "Primary4Y", Primary4Y::Id },
    { "Primary4Intensity", Primary4Intensity::Id },
    { "Primary5X", Primary5X::Id },
    { "Primary5Y", Primary5Y::Id },
    { "Primary5Intensity", Primary5Intensity::Id },
    { "Primary6X", Primary6X::Id },
    { "Primary6Y", Primary6Y::Id },
    { "Primary6Intensity", Primary6Intensity::Id },
    { "WhitePointX", WhitePointX::Id },
    { "WhitePointY", WhitePointY::Id },
    { "ColorPointRX", ColorPointRX::Id },
    { "ColorPointRY", ColorPointRY::Id },
    { "ColorPointRIntensity", ColorPointRIntensity::Id },
    { "ColorPointGX", ColorPointGX::Id },
    { "ColorPointGY", ColorPointGY::Id },
    { "ColorPointGIntensity", ColorPointGIntensity::Id },
    { "ColorPointBX", ColorPointBX::Id },
    { "ColorPointBY", ColorPointBY::Id },
    { "ColorPointBIntensity", ColorPointBIntensity::Id },
    { "EnhancedCurrentHue", EnhancedCurrentHue::Id },
    { "EnhancedColorMode", EnhancedColorMode::Id },
    { "ColorLoopActive", ColorLoopActive::Id },
    { "ColorLoopDirection", ColorLoopDirection::Id },
    { "ColorLoopTime", ColorLoopTime::Id },
    { "ColorLoopStartEnhancedHue", ColorLoopStartEnhancedHue::Id },
    { "ColorLoopStoredEnhancedHue", ColorLoopStoredEnhancedHue::Id },
    { "ColorCapabilities", ColorCapabilities::Id },
    { "ColorTempPhysicalMinMireds", ColorTempPhysicalMinMireds::Id },
    { "ColorTempPhysicalMaxMireds", ColorTempPhysicalMaxMireds::Id },
    { "CoupleColorTempToLevelMinMireds", CoupleColorTempToLevelMinMireds::Id },
    { "StartUpColorTemperatureMireds", StartUpColorTemperatureMireds::Id }
};
} // namespace Attributes
} // namespace ColorControl
namespace IlluminanceMeasurement {
namespace Attributes {
static const std::unordered_map<std::string, chip::AttributeId> unify_to_matter_attribute_id_cluster_map {
    { "MeasuredValue", MeasuredValue::Id },
    { "MinMeasuredValue", MinMeasuredValue::Id },
    { "MaxMeasuredValue", MaxMeasuredValue::Id },
    { "Tolerance", Tolerance::Id },
    { "LightSensorType", LightSensorType::Id }
};
} // namespace Attributes
} // namespace IlluminanceMeasurement
namespace TemperatureMeasurement {
namespace Attributes {
static const std::unordered_map<std::string, chip::AttributeId> unify_to_matter_attribute_id_cluster_map {
    { "MeasuredValue", MeasuredValue::Id },
    { "MinMeasuredValue", MinMeasuredValue::Id },
    { "MaxMeasuredValue", MaxMeasuredValue::Id },
    { "Tolerance", Tolerance::Id }
};
} // namespace Attributes
} // namespace TemperatureMeasurement
namespace PressureMeasurement {
namespace Attributes {
static const std::unordered_map<std::string, chip::AttributeId> unify_to_matter_attribute_id_cluster_map {
    { "MeasuredValue", MeasuredValue::Id },
    { "MinMeasuredValue", MinMeasuredValue::Id },
    { "MaxMeasuredValue", MaxMeasuredValue::Id },
    { "Tolerance", Tolerance::Id },
    { "ScaledValue", ScaledValue::Id },
    { "MinScaledValue", MinScaledValue::Id },
    { "MaxScaledValue", MaxScaledValue::Id },
    { "ScaledTolerance", ScaledTolerance::Id },
    { "Scale", Scale::Id }
};
} // namespace Attributes
} // namespace PressureMeasurement
namespace FlowMeasurement {
namespace Attributes {
static const std::unordered_map<std::string, chip::AttributeId> unify_to_matter_attribute_id_cluster_map {
    { "MeasuredValue", MeasuredValue::Id },
    { "MinMeasuredValue", MinMeasuredValue::Id },
    { "MaxMeasuredValue", MaxMeasuredValue::Id },
    { "Tolerance", Tolerance::Id }
};
} // namespace Attributes
} // namespace FlowMeasurement
namespace OccupancySensing {
namespace Attributes {
static const std::unordered_map<std::string, chip::AttributeId> unify_to_matter_attribute_id_cluster_map {
    { "Occupancy", Occupancy::Id },
    { "OccupancySensorType", OccupancySensorType::Id },
    { "OccupancySensorTypeBitmap", OccupancySensorTypeBitmap::Id },
    { "PirOccupiedToUnoccupiedDelay", PirOccupiedToUnoccupiedDelay::Id },
    { "PirUnoccupiedToOccupiedDelay", PirUnoccupiedToOccupiedDelay::Id },
    { "PirUnoccupiedToOccupiedThreshold", PirUnoccupiedToOccupiedThreshold::Id },
    { "UltrasonicOccupiedToUnoccupiedDelay", UltrasonicOccupiedToUnoccupiedDelay::Id },
    { "UltrasonicUnoccupiedToOccupiedDelay", UltrasonicUnoccupiedToOccupiedDelay::Id },
    { "UltrasonicUnoccupiedToOccupiedThreshold", UltrasonicUnoccupiedToOccupiedThreshold::Id },
    { "PhysicalContactOccupiedToUnoccupiedDelay", PhysicalContactOccupiedToUnoccupiedDelay::Id },
    { "PhysicalContactUnoccupiedToOccupiedDelay", PhysicalContactUnoccupiedToOccupiedDelay::Id },
    { "PhysicalContactUnoccupiedToOccupiedThreshold", PhysicalContactUnoccupiedToOccupiedThreshold::Id }
};
} // namespace Attributes
} // namespace OccupancySensing

// Global map of each clusters attributes
static const std::unordered_map<std::string, std::unordered_map<std::string, chip::AttributeId>> unify_to_matter_attribute_id_map {
    { "Basic",  BridgedDeviceBasic::Attributes::unify_to_matter_attribute_id_cluster_map },
    { "Identify",  Identify::Attributes::unify_to_matter_attribute_id_cluster_map },
    { "Groups",  Groups::Attributes::unify_to_matter_attribute_id_cluster_map },
    { "Scenes",  Scenes::Attributes::unify_to_matter_attribute_id_cluster_map },
    { "OnOff",  OnOff::Attributes::unify_to_matter_attribute_id_cluster_map },
    { "Level",  LevelControl::Attributes::unify_to_matter_attribute_id_cluster_map },
    { "FanControl",  FanControl::Attributes::unify_to_matter_attribute_id_cluster_map },
    { "ColorControl",  ColorControl::Attributes::unify_to_matter_attribute_id_cluster_map },
    { "IlluminanceMeasurement",  IlluminanceMeasurement::Attributes::unify_to_matter_attribute_id_cluster_map },
    { "TemperatureMeasurement",  TemperatureMeasurement::Attributes::unify_to_matter_attribute_id_cluster_map },
    { "PressureMeasurement",  PressureMeasurement::Attributes::unify_to_matter_attribute_id_cluster_map },
    { "FlowMeasurement",  FlowMeasurement::Attributes::unify_to_matter_attribute_id_cluster_map },
    { "OccupancySensing",  OccupancySensing::Attributes::unify_to_matter_attribute_id_cluster_map },
};


// Command mapping Unify->Matter conversion
// Command structure for each cluster
namespace BridgedDeviceBasic {
namespace Commands {
static const std::unordered_map<std::string, chip::CommandId> unify_to_matter_command_id_cluster_map {
};
} // namespace Commands
} // namespace Basic
namespace Identify {
namespace Commands {
static const std::unordered_map<std::string, chip::CommandId> unify_to_matter_command_id_cluster_map {
    { "Identify", Identify::Id },
    { "TriggerEffect", TriggerEffect::Id }
};
} // namespace Commands
} // namespace Identify
namespace Groups {
namespace Commands {
static const std::unordered_map<std::string, chip::CommandId> unify_to_matter_command_id_cluster_map {
    { "AddGroup", AddGroup::Id },
    { "ViewGroup", ViewGroup::Id },
    { "GetGroupMembership", GetGroupMembership::Id },
    { "RemoveGroup", RemoveGroup::Id },
    { "RemoveAllGroups", RemoveAllGroups::Id },
    { "AddGroupIfIdentifying", AddGroupIfIdentifying::Id }
};
} // namespace Commands
} // namespace Groups
namespace Scenes {
namespace Commands {
static const std::unordered_map<std::string, chip::CommandId> unify_to_matter_command_id_cluster_map {
    { "AddScene", AddScene::Id },
    { "ViewScene", ViewScene::Id },
    { "RemoveScene", RemoveScene::Id },
    { "RemoveAllScenes", RemoveAllScenes::Id },
    { "StoreScene", StoreScene::Id },
    { "RecallScene", RecallScene::Id },
    { "GetSceneMembership", GetSceneMembership::Id },
    { "EnhancedAddScene", EnhancedAddScene::Id },
    { "EnhancedViewScene", EnhancedViewScene::Id },
    { "CopyScene", CopyScene::Id },
};
} // namespace Commands
} // namespace Scenes
namespace OnOff {
namespace Commands {
static const std::unordered_map<std::string, chip::CommandId> unify_to_matter_command_id_cluster_map {
    { "Off", Off::Id },
    { "On", On::Id },
    { "Toggle", Toggle::Id },
    { "OffWithEffect", OffWithEffect::Id },
    { "OnWithRecallGlobalScene", OnWithRecallGlobalScene::Id },
    { "OnWithTimedOff", OnWithTimedOff::Id }
};
} // namespace Commands
} // namespace OnOff
namespace LevelControl {
namespace Commands {
static const std::unordered_map<std::string, chip::CommandId> unify_to_matter_command_id_cluster_map {
    { "MoveToLevel", MoveToLevel::Id },
    { "Move", Move::Id },
    { "Step", Step::Id },
    { "Stop", Stop::Id },
    { "MoveToLevelWithOnOff", MoveToLevelWithOnOff::Id },
    { "MoveWithOnOff", MoveWithOnOff::Id },
    { "StepWithOnOff", StepWithOnOff::Id },
    { "StopWithOnOff", StopWithOnOff::Id },
};
} // namespace Commands
} // namespace Level
namespace ColorControl {
namespace Commands {
static const std::unordered_map<std::string, chip::CommandId> unify_to_matter_command_id_cluster_map {
    { "MoveToHue", MoveToHue::Id },
    { "MoveHue", MoveHue::Id },
    { "StepHue", StepHue::Id },
    { "MoveToSaturation", MoveToSaturation::Id },
    { "MoveSaturation", MoveSaturation::Id },
    { "StepSaturation", StepSaturation::Id },
    { "MoveToHueAndSaturation", MoveToHueAndSaturation::Id },
    { "MoveToColor", MoveToColor::Id },
    { "MoveColor", MoveColor::Id },
    { "StepColor", StepColor::Id },
    { "MoveToColorTemperature", MoveToColorTemperature::Id },
    { "EnhancedMoveToHue", EnhancedMoveToHue::Id },
    { "EnhancedMoveHue", EnhancedMoveHue::Id },
    { "EnhancedStepHue", EnhancedStepHue::Id },
    { "EnhancedMoveToHueAndSaturation", EnhancedMoveToHueAndSaturation::Id },
    { "ColorLoopSet", ColorLoopSet::Id },
    { "StopMoveStep", StopMoveStep::Id },
    { "MoveColorTemperature", MoveColorTemperature::Id },
    { "StepColorTemperature", StepColorTemperature::Id }
};
} // namespace Commands
} // namespace ColorControl

// Global map of each clusters commands
static const std::unordered_map<std::string, std::unordered_map<std::string, chip::CommandId>> unify_to_matter_command_id_map {
    { "Basic",  BridgedDeviceBasic::Commands::unify_to_matter_command_id_cluster_map },
    { "Identify",  Identify::Commands::unify_to_matter_command_id_cluster_map },
    { "Groups",  Groups::Commands::unify_to_matter_command_id_cluster_map },
    { "Scenes",  Scenes::Commands::unify_to_matter_command_id_cluster_map },
    { "OnOff",  OnOff::Commands::unify_to_matter_command_id_cluster_map },
    { "Level",  LevelControl::Commands::unify_to_matter_command_id_cluster_map },
    { "ColorControl",  ColorControl::Commands::unify_to_matter_command_id_cluster_map },
};

} // namespace Clusters
} // namespace app
} // namespace chip
