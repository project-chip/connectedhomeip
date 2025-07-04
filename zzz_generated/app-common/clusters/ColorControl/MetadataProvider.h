// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ColorControl (cluster code: 768/0x300)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/ColorControl/Ids.h>
#include <clusters/ColorControl/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::ColorControl::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::ColorControl::Attributes;
        switch (attributeId)
        {
        case CurrentHue::Id:
            return CurrentHue::kMetadataEntry;
        case CurrentSaturation::Id:
            return CurrentSaturation::kMetadataEntry;
        case RemainingTime::Id:
            return RemainingTime::kMetadataEntry;
        case CurrentX::Id:
            return CurrentX::kMetadataEntry;
        case CurrentY::Id:
            return CurrentY::kMetadataEntry;
        case DriftCompensation::Id:
            return DriftCompensation::kMetadataEntry;
        case CompensationText::Id:
            return CompensationText::kMetadataEntry;
        case ColorTemperatureMireds::Id:
            return ColorTemperatureMireds::kMetadataEntry;
        case ColorMode::Id:
            return ColorMode::kMetadataEntry;
        case Options::Id:
            return Options::kMetadataEntry;
        case NumberOfPrimaries::Id:
            return NumberOfPrimaries::kMetadataEntry;
        case Primary1X::Id:
            return Primary1X::kMetadataEntry;
        case Primary1Y::Id:
            return Primary1Y::kMetadataEntry;
        case Primary1Intensity::Id:
            return Primary1Intensity::kMetadataEntry;
        case Primary2X::Id:
            return Primary2X::kMetadataEntry;
        case Primary2Y::Id:
            return Primary2Y::kMetadataEntry;
        case Primary2Intensity::Id:
            return Primary2Intensity::kMetadataEntry;
        case Primary3X::Id:
            return Primary3X::kMetadataEntry;
        case Primary3Y::Id:
            return Primary3Y::kMetadataEntry;
        case Primary3Intensity::Id:
            return Primary3Intensity::kMetadataEntry;
        case Primary4X::Id:
            return Primary4X::kMetadataEntry;
        case Primary4Y::Id:
            return Primary4Y::kMetadataEntry;
        case Primary4Intensity::Id:
            return Primary4Intensity::kMetadataEntry;
        case Primary5X::Id:
            return Primary5X::kMetadataEntry;
        case Primary5Y::Id:
            return Primary5Y::kMetadataEntry;
        case Primary5Intensity::Id:
            return Primary5Intensity::kMetadataEntry;
        case Primary6X::Id:
            return Primary6X::kMetadataEntry;
        case Primary6Y::Id:
            return Primary6Y::kMetadataEntry;
        case Primary6Intensity::Id:
            return Primary6Intensity::kMetadataEntry;
        case WhitePointX::Id:
            return WhitePointX::kMetadataEntry;
        case WhitePointY::Id:
            return WhitePointY::kMetadataEntry;
        case ColorPointRX::Id:
            return ColorPointRX::kMetadataEntry;
        case ColorPointRY::Id:
            return ColorPointRY::kMetadataEntry;
        case ColorPointRIntensity::Id:
            return ColorPointRIntensity::kMetadataEntry;
        case ColorPointGX::Id:
            return ColorPointGX::kMetadataEntry;
        case ColorPointGY::Id:
            return ColorPointGY::kMetadataEntry;
        case ColorPointGIntensity::Id:
            return ColorPointGIntensity::kMetadataEntry;
        case ColorPointBX::Id:
            return ColorPointBX::kMetadataEntry;
        case ColorPointBY::Id:
            return ColorPointBY::kMetadataEntry;
        case ColorPointBIntensity::Id:
            return ColorPointBIntensity::kMetadataEntry;
        case EnhancedCurrentHue::Id:
            return EnhancedCurrentHue::kMetadataEntry;
        case EnhancedColorMode::Id:
            return EnhancedColorMode::kMetadataEntry;
        case ColorLoopActive::Id:
            return ColorLoopActive::kMetadataEntry;
        case ColorLoopDirection::Id:
            return ColorLoopDirection::kMetadataEntry;
        case ColorLoopTime::Id:
            return ColorLoopTime::kMetadataEntry;
        case ColorLoopStartEnhancedHue::Id:
            return ColorLoopStartEnhancedHue::kMetadataEntry;
        case ColorLoopStoredEnhancedHue::Id:
            return ColorLoopStoredEnhancedHue::kMetadataEntry;
        case ColorCapabilities::Id:
            return ColorCapabilities::kMetadataEntry;
        case ColorTempPhysicalMinMireds::Id:
            return ColorTempPhysicalMinMireds::kMetadataEntry;
        case ColorTempPhysicalMaxMireds::Id:
            return ColorTempPhysicalMaxMireds::kMetadataEntry;
        case CoupleColorTempToLevelMinMireds::Id:
            return CoupleColorTempToLevelMinMireds::kMetadataEntry;
        case StartUpColorTemperatureMireds::Id:
            return StartUpColorTemperatureMireds::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::ColorControl::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::ColorControl::Commands;
        switch (commandId)
        {
        case MoveToHue::Id:
            return MoveToHue::kMetadataEntry;
        case MoveHue::Id:
            return MoveHue::kMetadataEntry;
        case StepHue::Id:
            return StepHue::kMetadataEntry;
        case MoveToSaturation::Id:
            return MoveToSaturation::kMetadataEntry;
        case MoveSaturation::Id:
            return MoveSaturation::kMetadataEntry;
        case StepSaturation::Id:
            return StepSaturation::kMetadataEntry;
        case MoveToHueAndSaturation::Id:
            return MoveToHueAndSaturation::kMetadataEntry;
        case MoveToColor::Id:
            return MoveToColor::kMetadataEntry;
        case MoveColor::Id:
            return MoveColor::kMetadataEntry;
        case StepColor::Id:
            return StepColor::kMetadataEntry;
        case MoveToColorTemperature::Id:
            return MoveToColorTemperature::kMetadataEntry;
        case EnhancedMoveToHue::Id:
            return EnhancedMoveToHue::kMetadataEntry;
        case EnhancedMoveHue::Id:
            return EnhancedMoveHue::kMetadataEntry;
        case EnhancedStepHue::Id:
            return EnhancedStepHue::kMetadataEntry;
        case EnhancedMoveToHueAndSaturation::Id:
            return EnhancedMoveToHueAndSaturation::kMetadataEntry;
        case ColorLoopSet::Id:
            return ColorLoopSet::kMetadataEntry;
        case StopMoveStep::Id:
            return StopMoveStep::kMetadataEntry;
        case MoveColorTemperature::Id:
            return MoveColorTemperature::kMetadataEntry;
        case StepColorTemperature::Id:
            return StepColorTemperature::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
