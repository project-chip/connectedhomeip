/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <algorithm>

#include "color-control-server.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <lib/core/Optional.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
#include <tracing/macros.h>

#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
#include <app/clusters/scenes-server/scenes-server.h>
#endif

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ColorControl;
using chip::Protocols::InteractionModel::Status;

#if defined(MATTER_DM_PLUGIN_SCENES_MANAGEMENT) && CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS
class DefaultColorControlSceneHandler : public scenes::DefaultSceneHandlerImpl
{
public:
    // As per spec, 9 attributes are scenable in the color control cluster, if new scenables attributes are added, this value should
    // be updated.
    static constexpr uint8_t kColorControlScenableAttributesCount = 9;

    DefaultColorControlSceneHandler() = default;
    ~DefaultColorControlSceneHandler() override {}

    // Default function for ColorControl cluster, only puts the ColorControl cluster ID in the span if supported on the caller
    // endpoint
    void GetSupportedClusters(EndpointId endpoint, Span<ClusterId> & clusterBuffer) override
    {
        ClusterId * buffer = clusterBuffer.data();
        if (emberAfContainsServer(endpoint, ColorControl::Id) && clusterBuffer.size() >= 1)
        {
            buffer[0] = ColorControl::Id;
            clusterBuffer.reduce_size(1);
        }
        else
        {
            clusterBuffer.reduce_size(0);
        }
    }

    // Default function for ColorControl cluster, only checks if ColorControl is enabled on the endpoint
    bool SupportsCluster(EndpointId endpoint, ClusterId cluster) override
    {
        return (cluster == ColorControl::Id) && (emberAfContainsServer(endpoint, ColorControl::Id));
    }

    /// @brief Serialize the Cluster's EFS value
    /// @param endpoint target endpoint
    /// @param cluster  target cluster
    /// @param serialisedBytes data to serialize into EFS
    /// @return CHIP_NO_ERROR if successfully serialized the data, CHIP_ERROR_INVALID_ARGUMENT otherwise
    CHIP_ERROR SerializeSave(EndpointId endpoint, ClusterId cluster, MutableByteSpan & serializedBytes) override
    {
        using AttributeValuePair = ScenesManagement::Structs::AttributeValuePairStruct::Type;

        AttributeValuePair pairs[kColorControlScenableAttributesCount];

        size_t attributeCount = 0;

        if (ColorControlServer::Instance().HasFeature(endpoint, ColorControlServer::Feature::kXy))
        {
            uint16_t xValue;
            if (Status::Success != Attributes::CurrentX::Get(endpoint, &xValue))
            {
                xValue = 0x616B; // Default X value according to spec
            }
            AddAttributeValuePair<uint16_t>(pairs, Attributes::CurrentX::Id, xValue, attributeCount);

            uint16_t yValue;
            if (Status::Success != Attributes::CurrentY::Get(endpoint, &yValue))
            {
                yValue = 0x607D; // Default Y value according to spec
            }
            AddAttributeValuePair<uint16_t>(pairs, Attributes::CurrentY::Id, yValue, attributeCount);
        }

        if (ColorControlServer::Instance().HasFeature(endpoint, ColorControlServer::Feature::kEnhancedHue))
        {
            uint16_t hueValue = 0x0000;
            Attributes::EnhancedCurrentHue::Get(endpoint, &hueValue);
            AddAttributeValuePair<uint16_t>(pairs, Attributes::EnhancedCurrentHue::Id, hueValue, attributeCount);
        }

        if (ColorControlServer::Instance().HasFeature(endpoint, ColorControlServer::Feature::kHueAndSaturation))
        {
            uint8_t saturationValue;
            if (Status::Success != Attributes::CurrentSaturation::Get(endpoint, &saturationValue))
            {
                saturationValue = 0x00;
            }
            AddAttributeValuePair<uint8_t>(pairs, Attributes::CurrentSaturation::Id, saturationValue, attributeCount);
        }

        if (ColorControlServer::Instance().HasFeature(endpoint, ColorControlServer::Feature::kColorLoop))
        {
            uint8_t loopActiveValue;
            if (Status::Success != Attributes::ColorLoopActive::Get(endpoint, &loopActiveValue))
            {
                loopActiveValue = 0x00;
            }
            AddAttributeValuePair<uint8_t>(pairs, Attributes::ColorLoopActive::Id, loopActiveValue, attributeCount);

            uint8_t loopDirectionValue;
            if (Status::Success != Attributes::ColorLoopDirection::Get(endpoint, &loopDirectionValue))
            {
                loopDirectionValue = 0x00;
            }
            AddAttributeValuePair<uint8_t>(pairs, Attributes::ColorLoopDirection::Id, loopDirectionValue, attributeCount);

            uint16_t loopTimeValue;
            if (Status::Success != Attributes::ColorLoopTime::Get(endpoint, &loopTimeValue))
            {
                loopTimeValue = 0x0019; // Default loop time value according to spec
            }
            AddAttributeValuePair<uint16_t>(pairs, Attributes::ColorLoopTime::Id, loopTimeValue, attributeCount);
        }

        if (ColorControlServer::Instance().HasFeature(endpoint, ColorControlServer::Feature::kColorTemperature))
        {
            uint16_t temperatureValue;
            if (Status::Success != Attributes::ColorTemperatureMireds::Get(endpoint, &temperatureValue))
            {
                temperatureValue = 0x00FA; // Default temperature value according to spec
            }
            AddAttributeValuePair<uint16_t>(pairs, Attributes::ColorTemperatureMireds::Id, temperatureValue, attributeCount);
        }

        EnhancedColorMode modeValue;
        if (Status::Success != Attributes::EnhancedColorMode::Get(endpoint, &modeValue))
        {
            modeValue = EnhancedColorMode::kCurrentXAndCurrentY; // Default mode value according to spec
        }
        AddAttributeValuePair(pairs, Attributes::EnhancedColorMode::Id, to_underlying(modeValue), attributeCount);

        app::DataModel::List<AttributeValuePair> attributeValueList(pairs, attributeCount);

        return EncodeAttributeValueList(attributeValueList, serializedBytes);
    }

    /// @brief Default EFS interaction when applying scene to the ColorControl Cluster
    /// @param endpoint target endpoint
    /// @param cluster  target cluster
    /// @param serialisedBytes Data from nvm
    /// @param timeMs transition time in ms
    /// @return CHIP_NO_ERROR if value as expected, CHIP_ERROR_INVALID_ARGUMENT otherwise
    CHIP_ERROR ApplyScene(EndpointId endpoint, ClusterId cluster, const ByteSpan & serializedBytes,
                          scenes::TransitionTimeMs timeMs) override
    {
        app::DataModel::DecodableList<ScenesManagement::Structs::AttributeValuePairStruct::DecodableType> attributeValueList;

        ReturnErrorOnFailure(DecodeAttributeValueList(serializedBytes, attributeValueList));

        size_t attributeCount = 0;
        auto pair_iterator    = attributeValueList.begin();

        // The color control cluster should have a maximum of 9 scenable attributes
        ReturnErrorOnFailure(attributeValueList.ComputeSize(&attributeCount));
        VerifyOrReturnError(attributeCount <= kColorControlScenableAttributesCount, CHIP_ERROR_BUFFER_TOO_SMALL);

        uint16_t epIndex = ColorControlServer::Instance().getEndpointIndex(endpoint);
        // Retrieve the buffers for different modes
#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV
        ColorControlServer::ColorHueTransitionState * colorHueTransitionState =
            ColorControlServer::Instance().getColorHueTransitionStateByIndex(epIndex);
        ColorControlServer::Color16uTransitionState * colorSaturationTransitionState =
            ColorControlServer::Instance().getSaturationTransitionStateByIndex(epIndex);
#endif
#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_XY
        ColorControlServer::Color16uTransitionState * colorXTransitionState =
            ColorControlServer::Instance().getXTransitionStateByIndex(epIndex);
        ColorControlServer::Color16uTransitionState * colorYTransitionState =
            ColorControlServer::Instance().getYTransitionStateByIndex(epIndex);
#endif
#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP
        ColorControlServer::Color16uTransitionState * colorTempTransitionState =
            ColorControlServer::Instance().getTempTransitionStateByIndex(epIndex);
#endif

        // Initialize action attributes to default values in case they are not in the scene
        auto targetColorMode       = EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation;
        uint8_t loopActiveValue    = 0x00;
        uint8_t loopDirectionValue = 0x00;
        uint16_t loopTimeValue     = 0x0019; // Default loop time value according to spec

        while (pair_iterator.Next())
        {
            auto & decodePair = pair_iterator.GetValue();

            switch (decodePair.attributeID)
            {
            case Attributes::CurrentX::Id:
                if (SupportsColorMode(endpoint, EnhancedColorMode::kCurrentXAndCurrentY))
                {
                    VerifyOrReturnError(decodePair.valueUnsigned16.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
                    colorXTransitionState->finalValue =
                        std::min(decodePair.valueUnsigned16.Value(), colorXTransitionState->highLimit);
                }
                break;
            case Attributes::CurrentY::Id:
                if (SupportsColorMode(endpoint, EnhancedColorMode::kCurrentXAndCurrentY))
                {
                    VerifyOrReturnError(decodePair.valueUnsigned16.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
                    colorYTransitionState->finalValue =
                        std::min(decodePair.valueUnsigned16.Value(), colorYTransitionState->highLimit);
                }
                break;
            case Attributes::EnhancedCurrentHue::Id:
                if (SupportsColorMode(endpoint, EnhancedColorMode::kEnhancedCurrentHueAndCurrentSaturation))
                {
                    VerifyOrReturnError(decodePair.valueUnsigned16.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
                    colorHueTransitionState->finalEnhancedHue = decodePair.valueUnsigned16.Value();
                }
                break;
            case Attributes::CurrentSaturation::Id:
                if (SupportsColorMode(endpoint, EnhancedColorMode::kCurrentHueAndCurrentSaturation))
                {
                    VerifyOrReturnError(decodePair.valueUnsigned8.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
                    colorSaturationTransitionState->finalValue = std::min(static_cast<uint16_t>(decodePair.valueUnsigned8.Value()),
                                                                          colorSaturationTransitionState->highLimit);
                }
                break;
            case Attributes::ColorLoopActive::Id:
                VerifyOrReturnError(decodePair.valueUnsigned8.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
                loopActiveValue = decodePair.valueUnsigned8.Value();
                break;
            case Attributes::ColorLoopDirection::Id:
                VerifyOrReturnError(decodePair.valueUnsigned8.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
                loopDirectionValue = decodePair.valueUnsigned8.Value();
                break;
            case Attributes::ColorLoopTime::Id:
                VerifyOrReturnError(decodePair.valueUnsigned16.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
                loopTimeValue = decodePair.valueUnsigned16.Value();
                break;
            case Attributes::ColorTemperatureMireds::Id:
                if (SupportsColorMode(endpoint, EnhancedColorMode::kColorTemperatureMireds))
                {
                    VerifyOrReturnError(decodePair.valueUnsigned16.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
                    colorTempTransitionState->finalValue =
                        std::min(decodePair.valueUnsigned16.Value(), colorTempTransitionState->highLimit);
                }
                break;
            case Attributes::EnhancedColorMode::Id:
                VerifyOrReturnError(decodePair.valueUnsigned8.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
                if (decodePair.valueUnsigned8.Value() <= to_underlying(EnhancedColorMode::kEnhancedCurrentHueAndCurrentSaturation))
                {
                    targetColorMode = static_cast<EnhancedColorModeEnum>(decodePair.valueUnsigned8.Value());
                }
                break;
            default:
                return CHIP_ERROR_INVALID_ARGUMENT;
                break;
            }
        }
        ReturnErrorOnFailure(pair_iterator.GetStatus());

        // Switch to the mode saved in the scene
        if (SupportsColorMode(endpoint, targetColorMode))
        {
            ColorControlServer::Instance().handleModeSwitch(endpoint, targetColorMode);
        }
        else
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        uint16_t transitionTime10th = static_cast<uint16_t>(timeMs / 100);

        if (loopActiveValue == 1 && ColorControlServer::Instance().HasFeature(endpoint, ColorControlServer::Feature::kColorLoop))
        {
            // Set Loop Scene Attributes and start loop if scene stored active loop
            Attributes::ColorLoopDirection::Set(endpoint, loopDirectionValue);
            Attributes::ColorLoopTime::Set(endpoint, loopTimeValue);
            // Tries to apply color control loop
            ColorControlServer::Instance().startColorLoop(endpoint, true);
        }
        else
        {
            // Execute movement to value depending on the mode in the saved scene
            switch (targetColorMode)
            {
            case EnhancedColorMode::kCurrentHueAndCurrentSaturation:
#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV
                ColorControlServer::Instance().moveToSaturation(
                    endpoint, static_cast<uint8_t>(colorSaturationTransitionState->finalValue), transitionTime10th);
#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV
                break;
            case EnhancedColorMode::kCurrentXAndCurrentY:
#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_XY
                ColorControlServer::Instance().moveToColor(endpoint, colorXTransitionState->finalValue,
                                                           colorYTransitionState->finalValue, transitionTime10th);
#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_XY
                break;
            case EnhancedColorMode::kColorTemperatureMireds:
#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP
                ColorControlServer::Instance().moveToColorTemp(
                    endpoint, static_cast<uint16_t>(colorTempTransitionState->finalValue), transitionTime10th);
#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP
                break;
            case EnhancedColorMode::kEnhancedCurrentHueAndCurrentSaturation:
#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV
                ColorControlServer::Instance().moveToHueAndSaturation(
                    endpoint, colorHueTransitionState->finalEnhancedHue,
                    static_cast<uint8_t>(colorSaturationTransitionState->finalValue), transitionTime10th, true);
#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV
                break;
            default:
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
        }
        return CHIP_NO_ERROR;
    }

private:
    bool SupportsColorMode(EndpointId endpoint, EnhancedColorMode mode)
    {
        switch (mode)
        {
        case EnhancedColorMode::kCurrentHueAndCurrentSaturation:
            return ColorControlServer::Instance().HasFeature(endpoint, ColorControlServer::Feature::kHueAndSaturation);
            break;
        case EnhancedColorMode::kCurrentXAndCurrentY:
            return ColorControlServer::Instance().HasFeature(endpoint, ColorControlServer::Feature::kXy);
            break;
        case EnhancedColorMode::kColorTemperatureMireds:
            return ColorControlServer::Instance().HasFeature(endpoint, ColorControlServer::Feature::kColorTemperature);
            break;
        case EnhancedColorMode::kEnhancedCurrentHueAndCurrentSaturation:
            return ColorControlServer::Instance().HasFeature(endpoint, ColorControlServer::Feature::kEnhancedHue);
            break;
        default:
            return false;
        }
    }

    /// AddAttributeValuePair
    /// @brief Helper function to add an attribute value pair to the attribute value pair array in the color control SceneHandler
    /// @param pairs list of attribute value pairs
    /// @param id attribute id
    /// @param value attribute value
    /// @param attributeCount number of attributes in the list, incremented by this function, used to keep track of how many
    /// attributes from the array are being used for the list to encode
    template <typename Type>
    void AddAttributeValuePair(ScenesManagement::Structs::AttributeValuePairStruct::Type * pairs, AttributeId id, Type value,
                               size_t & attributeCount)
    {
        static_assert((std::is_same_v<Type, uint8_t>) || (std::is_same_v<Type, uint16_t>), "Type must be uint8_t or uint16_t");

        pairs[attributeCount].attributeID = id;
        if constexpr ((std::is_same_v<Type, uint8_t>) )
        {
            pairs[attributeCount].valueUnsigned8.SetValue(value);
        }
        else if constexpr ((std::is_same_v<Type, uint16_t>) )
        {
            pairs[attributeCount].valueUnsigned16.SetValue(value);
        }
        attributeCount++;
    }
};
static DefaultColorControlSceneHandler sColorControlSceneHandler;
#endif // defined(MATTER_DM_PLUGIN_SCENES_MANAGEMENT) && CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS

/**********************************************************
 * Matter timer scheduling glue logic
 *********************************************************/

void ColorControlServer::timerCallback(System::Layer *, void * callbackContext)
{
    auto control = static_cast<EmberEventControl *>(callbackContext);
    (control->callback)(control->endpoint);
}

void ColorControlServer::scheduleTimerCallbackMs(EmberEventControl * control, uint32_t delayMs)
{
    CHIP_ERROR err = DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(delayMs), timerCallback, control);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Color Control Server failed to schedule event: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void ColorControlServer::cancelEndpointTimerCallback(EmberEventControl * control)
{
    DeviceLayer::SystemLayer().CancelTimer(timerCallback, control);
}

void ColorControlServer::cancelEndpointTimerCallback(EndpointId endpoint)
{
    auto control = ColorControlServer::getEventControl(endpoint);
    if (control)
    {
        cancelEndpointTimerCallback(control);
    }
}

/**********************************************************
 * Attributes Definition
 *********************************************************/

ColorControlServer ColorControlServer::instance;

/**********************************************************
 * ColorControl Implementation
 *********************************************************/

ColorControlServer & ColorControlServer::Instance()
{
    return instance;
}

uint16_t ColorControlServer::getEndpointIndex(EndpointId endpoint)
{
    return emberAfGetClusterServerEndpointIndex(endpoint, ColorControl::Id, MATTER_DM_COLOR_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
}

#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
chip::scenes::SceneHandler * ColorControlServer::GetSceneHandler()
{
#if CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS
    return &sColorControlSceneHandler;
#else
    return nullptr;
#endif // CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS
}
#endif // ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT

bool ColorControlServer::HasFeature(chip::EndpointId endpoint, Feature feature)
{
    bool success;
    uint32_t featureMap;
    success = (Attributes::FeatureMap::Get(endpoint, &featureMap) == Status::Success);

    return success ? ((featureMap & to_underlying(feature)) != 0) : false;
}

Status ColorControlServer::stopAllColorTransitions(EndpointId endpoint)
{
    EmberEventControl * event = getEventControl(endpoint);
    VerifyOrReturnError(event != nullptr, Status::UnsupportedEndpoint);

    cancelEndpointTimerCallback(event);
    return Status::Success;
}

Status ColorControlServer::stopMoveStepCommand(EndpointId endpoint, const Commands::StopMoveStep::DecodableType & commandData)
{
    Status status = Status::Success;

    // StopMoveStep command has no effect on an active color loop.
    // Fetch if it is supported and active.
    uint8_t isColorLoopActive = 0;
    if (ColorControlServer::Instance().HasFeature(endpoint, ColorControlServer::Feature::kColorLoop))
    {
        // In case of get failure, isColorLoopActive will remain at the init value 0 (not active)
        if (Attributes::ColorLoopActive::Get(endpoint, &isColorLoopActive) != Status::Success)
        {
            ChipLogError(Zcl, "Failed to retrieve ColorLoopActive value");
        }
    }

    if (shouldExecuteIfOff(endpoint, commandData.optionsMask, commandData.optionsOverride) && !isColorLoopActive)
    {
        status = stopAllColorTransitions(endpoint);

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV
        // Because Hue and Saturation have separate transitions and can be kicked separately,
        // a new command specific to Hue could resume an old unfinished Saturation transition. Or vice versa.
        // Init both transition states on stop command to prevent that.
        if (status == Status::Success)
        {
            uint16_t epIndex                          = getEndpointIndex(endpoint);
            ColorHueTransitionState * hueState        = getColorHueTransitionStateByIndex(epIndex);
            Color16uTransitionState * saturationState = getSaturationTransitionStateByIndex(epIndex);
            initHueTransitionState(endpoint, hueState, false /*isEnhancedHue don't care*/);
            initSaturationTransitionState(endpoint, saturationState);
        }
#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV
    }

    return status;
}

bool ColorControlServer::shouldExecuteIfOff(EndpointId endpoint, BitMask<OptionsBitmap> optionMask,
                                            BitMask<OptionsBitmap> optionOverride)
{
    // From 5.2.2.2.1.10 of ZCL7 document 14-0129-15f-zcl-ch-5-lighting.docx:
    //   "Command execution SHALL NOT continue beyond the Options processing if
    //    all of these criteria are true:
    //      - The On/Off cluster exists on the same endpoint as this cluster.
    //      - The OnOff attribute of the On/Off cluster, on this endpoint, is 0x00
    //        (FALSE).
    //      - The value of the ExecuteIfOff bit is 0."

    if (!emberAfContainsServer(endpoint, OnOff::Id))
    {
        return true;
    }

    BitMask<OptionsBitmap> options = 0x00;
    Attributes::Options::Get(endpoint, &options);

    bool on = true;
    OnOff::Attributes::OnOff::Get(endpoint, &on);

    // The device is on - hence ExecuteIfOff does not matter
    if (on)
    {
        return true;
    }
    // The OptionsMask & OptionsOverride fields SHALL both be present or both
    // omitted in the command. A temporary Options bitmap SHALL be created from
    // the Options attribute, using the OptionsMask & OptionsOverride fields, if
    // present. Each bit of the temporary Options bitmap SHALL be determined as
    // follows:
    // Each bit in the Options attribute SHALL determine the corresponding bit in
    // the temporary Options bitmap, unless the OptionsMask field is present and
    // has the corresponding bit set to 1, in which case the corresponding bit in
    // the OptionsOverride field SHALL determine the corresponding bit in the
    // temporary Options bitmap.
    // The resulting temporary Options bitmap SHALL then be processed as defined
    // in section 5.2.2.2.1.10.

    // ---------- The following order is important in decision making -------
    // -----------more readable ----------
    //
    if (optionMask == static_cast<OptionsBitmap>(0xFF) && optionOverride == static_cast<OptionsBitmap>(0xFF))
    {
        // 0xFF are the default values passed to the command handler when
        // the payload is not present - in that case there is use of option
        // attribute to decide execution of the command
        return options.Has(OptionsBitmap::kExecuteIfOff);
    }
    // ---------- The above is to distinguish if the payload is present or not

    if (optionMask.Has(OptionsBitmap::kExecuteIfOff))
    {
        // Mask is present and set in the command payload, this indicates
        // use the override as temporary option
        return optionOverride.Has(OptionsBitmap::kExecuteIfOff);
    }
    // if we are here - use the option bits
    return options.Has(OptionsBitmap::kExecuteIfOff);
}

/**
 * @brief The specification says that if we are transitioning from one color mode
 * into another, we need to compute the new mode's attribute values from the
 * old mode.  However, it also says that if the old mode doesn't translate into
 * the new mode, this must be avoided.
 * I am putting in this function to compute the new attributes based on the old
 * color mode.
 *
 * @param endpoint
 * @param newColorMode
 */
void ColorControlServer::handleModeSwitch(EndpointId endpoint, EnhancedColorModeEnum newColorMode)
{
    auto oldColorMode = ColorModeEnum::kCurrentHueAndCurrentSaturation;
    Attributes::ColorMode::Get(endpoint, &oldColorMode);

    uint8_t colorModeTransition;

    if (static_cast<EnhancedColorModeEnum>(oldColorMode) == newColorMode)
    {
        return;
    }

    Attributes::EnhancedColorMode::Set(endpoint, newColorMode);
    if (newColorMode == EnhancedColorMode::kEnhancedCurrentHueAndCurrentSaturation)
    {
        // Transpose COLOR_MODE_EHSV to EnhancedColorMode::kCurrentHueAndCurrentSaturation after setting
        // EnhancedColorMode
        newColorMode = EnhancedColorMode::kCurrentHueAndCurrentSaturation;
    }
    Attributes::ColorMode::Set(endpoint, static_cast<ColorModeEnum>(newColorMode));

    colorModeTransition = static_cast<uint8_t>((to_underlying(newColorMode) << 4) + to_underlying(oldColorMode));

    // Note:  It may be OK to not do anything here.
    switch (colorModeTransition)
    {
    case ColorControlServer::Conversion::HSV_TO_CIE_XY:
        computePwmFromXy(endpoint);
        break;
    case ColorControlServer::Conversion::TEMPERATURE_TO_CIE_XY:
        computePwmFromXy(endpoint);
        break;
    case ColorControlServer::Conversion::CIE_XY_TO_HSV:
        computePwmFromHsv(endpoint);
        break;
    case ColorControlServer::Conversion::TEMPERATURE_TO_HSV:
        computePwmFromHsv(endpoint);
        break;
    case ColorControlServer::Conversion::HSV_TO_TEMPERATURE:
        computePwmFromTemp(endpoint);
        break;
    case ColorControlServer::Conversion::CIE_XY_TO_TEMPERATURE:
        computePwmFromTemp(endpoint);
        break;

    // for the following cases, there is no transition.
    case ColorControlServer::Conversion::HSV_TO_HSV:
    case ColorControlServer::Conversion::CIE_XY_TO_CIE_XY:
    case ColorControlServer::Conversion::TEMPERATURE_TO_TEMPERATURE:
    default:
        return;
    }
}

/**
 * @brief calculates transition time frame currant sate and rate
 *
 * @param[in] p current Color16uTransitionState
 * @param[in] rate
 * @return uint16_t
 */
uint16_t ColorControlServer::computeTransitionTimeFromStateAndRate(ColorControlServer::Color16uTransitionState * p, uint16_t rate)
{
    uint32_t transitionTime;
    uint16_t max, min;

    if (rate == 0)
    {
        return MAX_INT16U_VALUE;
    }

    if (p->currentValue > p->finalValue)
    {
        max = p->currentValue;
        min = p->finalValue;
    }
    else
    {
        max = p->finalValue;
        min = p->currentValue;
    }

    transitionTime = max - min;
    transitionTime *= 10;
    transitionTime /= rate;

    // If transitionTime == 0, force 1 step
    transitionTime = transitionTime == 0 ? 1 : transitionTime;

    if (transitionTime > MAX_INT16U_VALUE)
    {
        return MAX_INT16U_VALUE;
    }

    return (uint16_t) transitionTime;
}

/**
 * @brief event control object for an endpoint
 *
 * @param[in] endpoint
 * @return EmberEventControl*
 */
EmberEventControl * ColorControlServer::getEventControl(EndpointId endpoint)
{
    uint16_t index            = getEndpointIndex(endpoint);
    EmberEventControl * event = nullptr;

    if (index < MATTER_ARRAY_SIZE(eventControls))
    {
        event = &eventControls[index];
    }
    return event;
}

/** @brief Compute Pwm from HSV
 *
 * This function is called from the color server when it is time for the PWMs to
 * be driven with a new value from the color temperature.
 *
 * @param endpoint The identifying endpoint Ver.: always
 */
void ColorControlServer::computePwmFromTemp(EndpointId endpoint) {}

/** @brief Compute Pwm from HSV
 *
 * This function is called from the color server when it is time for the PWMs to
 * be driven with a new value from the HSV values.
 *
 * @param endpoint The identifying endpoint Ver.: always
 */
void ColorControlServer::computePwmFromHsv(EndpointId endpoint) {}

/** @brief Compute Pwm from HSV
 *
 * This function is called from the color server when it is time for the PWMs to
 * be driven with a new value from the color X and color Y values.
 *
 * @param endpoint The identifying endpoint Ver.: always
 */
void ColorControlServer::computePwmFromXy(EndpointId endpoint) {}

/**
 * @brief Computes new color value based on current position
 *
 * @param p Color16uTransitionState*
 * @return true command movement is finished
 * @return false command movement is not finished
 */
bool ColorControlServer::computeNewColor16uValue(ColorControlServer::Color16uTransitionState * p)
{
    uint32_t newValue32u;

    // Color value isn't moving
    if (p->stepsRemaining == 0)
    {
        return true;
    }

    (p->stepsRemaining)--;

    if (p->timeRemaining > 0)
    {
        // The time remaining is measured in tenths of a second, which is the same as the update timer.
        (p->timeRemaining)--;
    }

    // handle sign
    if (p->finalValue == p->currentValue)
    {
        // do nothing
    }
    else if (p->finalValue > p->initialValue)
    {
        newValue32u = static_cast<uint32_t>(p->finalValue - p->initialValue);
        newValue32u *= static_cast<uint32_t>(p->stepsRemaining);

        /*
            stepsTotal should always be at least 1,
            still, prevent division by 0 and skips a meaningless division by 1
        */
        if (p->stepsTotal > 1)
        {
            newValue32u /= static_cast<uint32_t>(p->stepsTotal);
        }

        p->currentValue = static_cast<uint16_t>(p->finalValue - static_cast<uint16_t>(newValue32u));

        if (static_cast<uint16_t>(newValue32u) > p->finalValue || p->currentValue > p->highLimit)
        {
            p->currentValue = p->highLimit;
        }
    }
    else
    {
        newValue32u = static_cast<uint32_t>(p->initialValue - p->finalValue);
        newValue32u *= static_cast<uint32_t>(p->stepsRemaining);

        /*
            stepsTotal should always be at least 1,
            still, prevent division by 0 and skips a meaningless division by 1
        */
        if (p->stepsTotal > 1)
        {
            newValue32u /= static_cast<uint32_t>(p->stepsTotal);
        }

        p->currentValue = static_cast<uint16_t>(p->finalValue + static_cast<uint16_t>(newValue32u));

        if (p->finalValue > UINT16_MAX - static_cast<uint16_t>(newValue32u) || p->currentValue < p->lowLimit)
        {
            p->currentValue = p->lowLimit;
        }
    }

    if (p->stepsRemaining == 0)
    {
        // we have completed our move.
        return true;
    }

    return false;
}

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV

/**
 * @brief Returns ColorHueTransititionState associated to an endpoint index
 *
 * @param[in] endpoint
 * @return ColorControlServer::ColorHueTransitionState*
 */
ColorControlServer::ColorHueTransitionState * ColorControlServer::getColorHueTransitionStateByIndex(uint16_t index)
{
    ColorHueTransitionState * state = nullptr;

    if (index < MATTER_ARRAY_SIZE(colorHueTransitionStates))
    {
        state = &colorHueTransitionStates[index];
    }
    return state;
}

/**
 * @brief Returns ColorHueTransititionState associated to an endpoint
 *
 * @param[in] endpoint
 * @return ColorControlServer::ColorHueTransitionState*
 */
ColorControlServer::ColorHueTransitionState * ColorControlServer::getColorHueTransitionState(EndpointId endpoint)
{
    return getColorHueTransitionStateByIndex(getEndpointIndex(endpoint));
}

/**
 * @brief Returns the saturation Color16uTransitionState associated to an endpoint index
 *
 * @param[in] endpoint
 * @return ColorControlServer::Color16uTransitionState*
 */
ColorControlServer::Color16uTransitionState * ColorControlServer::getSaturationTransitionStateByIndex(uint16_t index)
{
    Color16uTransitionState * state = nullptr;

    if (index < MATTER_ARRAY_SIZE(colorSatTransitionStates))
    {
        state = &colorSatTransitionStates[index];
    }
    return state;
}

/**
 * @brief Returns the saturation Color16uTransitionState associated to an endpoint
 *
 * @param[in] endpoint
 * @return ColorControlServer::Color16uTransitionState*
 */
ColorControlServer::Color16uTransitionState * ColorControlServer::getSaturationTransitionState(EndpointId endpoint)
{
    return getSaturationTransitionStateByIndex(getEndpointIndex(endpoint));
}

/**
 * @brief Returns current saturation for a specified endpoint
 *
 * @param[in] endpoint
 * @return uint8_t
 */
uint8_t ColorControlServer::getSaturation(EndpointId endpoint)
{
    uint8_t saturation = 0;
    Attributes::CurrentSaturation::Get(endpoint, &saturation);

    return saturation;
}

/**
 * @brief Adds two hue values.
 * If the sum is bigger than max hue value, max hue value is returned
 *
 * @param[in] hue1
 * @param[in] hue2
 * @return uint8_t
 */
uint8_t ColorControlServer::addHue(uint8_t hue1, uint8_t hue2)
{
    uint16_t hue16;

    hue16 = ((uint16_t) hue1);
    hue16 = static_cast<uint16_t>(hue16 + static_cast<uint16_t>(hue2));

    if (hue16 > MAX_HUE_VALUE)
    {
        hue16 = static_cast<uint16_t>(hue16 - MAX_HUE_VALUE - 1);
    }

    return ((uint8_t) hue16);
}

/**
 * @brief Return difference between two hues.
 *
 * @param hue1
 * @param hue2
 * @return uint8_t
 */
uint8_t ColorControlServer::subtractHue(uint8_t hue1, uint8_t hue2)
{
    uint16_t hue16;

    hue16 = ((uint16_t) hue1);
    if (hue2 > hue1)
    {
        hue16 = static_cast<uint16_t>(hue16 + MAX_HUE_VALUE + 1);
    }

    hue16 = static_cast<uint16_t>(hue16 - static_cast<uint16_t>(hue2));

    return ((uint8_t) hue16);
}

/**
 * @brief Returns sum of two saturations. If Overflow, return max saturation value
 *
 * @param[in] saturation1
 * @param[in] saturation2
 * @return uint8_t
 */
uint8_t ColorControlServer::addSaturation(uint8_t saturation1, uint8_t saturation2)
{
    uint16_t saturation16;

    saturation16 = ((uint16_t) saturation1);
    saturation16 = static_cast<uint16_t>(saturation16 + static_cast<uint16_t>(saturation2));

    if (saturation16 > MAX_SATURATION_VALUE)
    {
        saturation16 = MAX_SATURATION_VALUE;
    }

    return ((uint8_t) saturation16);
}

/**
 * @brief Returns difference between two saturations. If Underflow, returns min saturation value
 *
 * @param saturation1
 * @param saturation2
 * @return uint8_t
 */
uint8_t ColorControlServer::subtractSaturation(uint8_t saturation1, uint8_t saturation2)
{
    if (saturation2 > saturation1)
    {
        return MIN_SATURATION_VALUE;
    }

    return static_cast<uint8_t>(saturation1 - saturation2);
}

/**
 * @brief Returns sum of two enhanced hues
 *
 * @param[in] hue1
 * @param[in] hue2
 * @return uint16_t
 */
uint16_t ColorControlServer::addEnhancedHue(uint16_t hue1, uint16_t hue2)
{
    return static_cast<uint16_t>(hue1 + hue2);
}

/**
 * @brief Returns difference of two enhanced hues
 *
 * @param[in] hue1
 * @param[in] hue2
 * @return uint16_t
 */
uint16_t ColorControlServer::subtractEnhancedHue(uint16_t hue1, uint16_t hue2)
{
    return static_cast<uint16_t>(hue1 - hue2);
}

/**
 * @brief Configures and launches color loop for a specified endpoint
 *
 * @param endpoint
 * @param startFromStartHue True, start from StartEnhancedHue attribute. False, start from currentEnhancedHue
 */
void ColorControlServer::startColorLoop(EndpointId endpoint, uint8_t startFromStartHue)
{
    ColorHueTransitionState * colorHueTransitionState = getColorHueTransitionState(endpoint);
    VerifyOrReturn(colorHueTransitionState != nullptr);

    uint8_t direction = 0;
    Attributes::ColorLoopDirection::Get(endpoint, &direction);

    uint16_t time = 0x0019;
    Attributes::ColorLoopTime::Get(endpoint, &time);

    uint16_t startHue = 0x2300;

    initHueTransitionState(endpoint, colorHueTransitionState, true /*isEnhancedHue Always true for colorLoop*/);
    Attributes::ColorLoopStoredEnhancedHue::Set(endpoint, colorHueTransitionState->currentEnhancedHue);
    Attributes::ColorLoopActive::Set(endpoint, true);

    if (startFromStartHue)
    {
        Attributes::ColorLoopStartEnhancedHue::Get(endpoint, &startHue);
    }
    else
    {
        startHue = colorHueTransitionState->currentEnhancedHue;
    }

    colorHueTransitionState->initialEnhancedHue = startHue;

    if (direction == to_underlying(ColorLoopDirectionEnum::kIncrement))
    {
        colorHueTransitionState->finalEnhancedHue = static_cast<uint16_t>(startHue - 1);
    }
    else
    {
        colorHueTransitionState->finalEnhancedHue = static_cast<uint16_t>(startHue + 1);
    }

    colorHueTransitionState->up     = (direction == to_underlying(ColorLoopDirectionEnum::kIncrement));
    colorHueTransitionState->repeat = true;

    colorHueTransitionState->stepsRemaining = static_cast<uint16_t>(time * TRANSITION_STEPS_PER_1S);
    colorHueTransitionState->stepsTotal     = static_cast<uint16_t>(time * TRANSITION_STEPS_PER_1S);
    colorHueTransitionState->timeRemaining  = MAX_INT16U_VALUE;
    colorHueTransitionState->transitionTime = MAX_INT16U_VALUE;
    colorHueTransitionState->endpoint       = endpoint;

    SetQuietReportRemainingTime(endpoint, MAX_INT16U_VALUE, true /* isNewTransition */);

    scheduleTimerCallbackMs(configureHSVEventControl(endpoint), TRANSITION_UPDATE_TIME_MS.count());
}

/**
 * @brief Initialise Hue and EnhancedHue TransitionState structure to begin a new transition
 *
 */
void ColorControlServer::initHueTransitionState(EndpointId endpoint, ColorHueTransitionState * colorHueTransitionState,
                                                bool isEnhancedHue)
{
    colorHueTransitionState->stepsRemaining = 0;
    colorHueTransitionState->timeRemaining  = 0;
    colorHueTransitionState->isEnhancedHue  = isEnhancedHue;
    colorHueTransitionState->endpoint       = endpoint;

    if (isEnhancedHue)
    {
        Attributes::EnhancedCurrentHue::Get(endpoint, &(colorHueTransitionState->currentEnhancedHue));
        colorHueTransitionState->initialEnhancedHue = colorHueTransitionState->currentEnhancedHue;
    }
    else
    {
        Attributes::CurrentHue::Get(endpoint, &(colorHueTransitionState->currentHue));
        colorHueTransitionState->initialHue = colorHueTransitionState->currentHue;
    }
}

/**
 * @brief Initialise Saturation Transition State structure to begin a new transition
 *
 */
void ColorControlServer::initSaturationTransitionState(chip::EndpointId endpoint, Color16uTransitionState * colorSatTransitionState)
{
    colorSatTransitionState->stepsRemaining = 0;
    colorSatTransitionState->timeRemaining  = 0;
    colorSatTransitionState->endpoint       = endpoint;

    colorSatTransitionState->initialValue = colorSatTransitionState->currentValue = getSaturation(endpoint);
}

void ColorControlServer::SetHSVRemainingTime(chip::EndpointId endpoint)
{
    uint16_t epIndex                                    = getEndpointIndex(endpoint);
    ColorHueTransitionState * hueTransitionState        = getColorHueTransitionStateByIndex(epIndex);
    Color16uTransitionState * saturationTransitionState = getSaturationTransitionStateByIndex(epIndex);

    // When the hue transition is loop, RemainingTime stays at MAX_INT16
    if (hueTransitionState->repeat == false)
    {
        bool hsvTransitionStart = (hueTransitionState->stepsRemaining == hueTransitionState->stepsTotal) ||
            (saturationTransitionState->stepsRemaining == saturationTransitionState->stepsTotal);
        SetQuietReportRemainingTime(endpoint, std::max(hueTransitionState->timeRemaining, saturationTransitionState->timeRemaining),
                                    hsvTransitionStart);
    }
}

/**
 * @brief Computes new hue value based on current position
 *
 * @param p ColorHueTransitionState*
 * @return true command movement is finished
 * @return false command movement is not finished
 */
bool ColorControlServer::computeNewHueValue(ColorControlServer::ColorHueTransitionState * p)
{
    uint32_t newHue32;
    uint16_t newHue;

    // hue is not currently moving
    if (p->stepsRemaining == 0)
    {
        return true;
    }

    (p->stepsRemaining)--;

    if (p->timeRemaining > 0 && p->repeat == false)
    {
        // The time remaining is measured in tenths of a second, which is the same as the update timer.
        (p->timeRemaining)--;
    }

    // are we going up or down?
    if ((p->isEnhancedHue && p->finalEnhancedHue == p->currentEnhancedHue) || (!p->isEnhancedHue && p->finalHue == p->currentHue))
    {
        // do nothing
    }
    else if (p->up)
    {
        newHue32 = static_cast<uint32_t>(p->isEnhancedHue ? subtractEnhancedHue(p->finalEnhancedHue, p->initialEnhancedHue)
                                                          : subtractHue(p->finalHue, p->initialHue));
        newHue32 *= static_cast<uint32_t>(p->stepsRemaining);

        /*
            stepsTotal should always be at least 1,
            still, prevent division by 0 and skips a meaningless division by 1
        */
        if (p->stepsTotal > 1)
        {
            newHue32 /= static_cast<uint32_t>(p->stepsTotal);
        }

        if (p->isEnhancedHue)
        {
            p->currentEnhancedHue = subtractEnhancedHue(p->finalEnhancedHue, static_cast<uint16_t>(newHue32));
        }
        else
        {
            p->currentHue = subtractHue(p->finalHue, static_cast<uint8_t>(newHue32));
        }
    }
    else
    {
        newHue32 = static_cast<uint32_t>(p->isEnhancedHue ? subtractEnhancedHue(p->initialEnhancedHue, p->finalEnhancedHue)
                                                          : subtractHue(p->initialHue, p->finalHue));
        newHue32 *= static_cast<uint32_t>(p->stepsRemaining);

        /*
            stepsTotal should always be at least 1,
            still, prevent division by 0 and skips a meaningless division by 1
        */
        if (p->stepsTotal > 1)
        {
            newHue32 /= static_cast<uint32_t>(p->stepsTotal);
        }

        if (p->isEnhancedHue)
        {
            p->currentEnhancedHue = addEnhancedHue(p->finalEnhancedHue, static_cast<uint16_t>(newHue32));
        }
        else
        {
            p->currentHue = addHue(p->finalHue, static_cast<uint8_t>(newHue32));
        }
    }

    if (p->stepsRemaining == 0)
    {
        if (p->repeat == false)
        {
            // we are performing a move to and not a move.
            return true;
        }

        // Check if we are in a color loop. If not, we are in a moveHue
        uint8_t isColorLoop = 0;
        Attributes::ColorLoopActive::Get(p->endpoint, &isColorLoop);

        if (isColorLoop)
        {
            p->currentEnhancedHue = p->initialEnhancedHue;
        }
        else
        {
            // we are performing a Hue         move.  Need to compute the new values for the
            // next move         period.
            if (p->up)
            {
                if (p->isEnhancedHue)
                {
                    newHue = subtractEnhancedHue(p->finalEnhancedHue, p->initialEnhancedHue);
                    newHue = addEnhancedHue(p->finalEnhancedHue, newHue);

                    p->initialEnhancedHue = p->finalEnhancedHue;
                    p->finalEnhancedHue   = newHue;
                }
                else
                {
                    newHue = subtractHue(p->finalHue, p->initialHue);
                    newHue = addHue(p->finalHue, static_cast<uint8_t>(newHue));

                    p->initialHue = p->finalHue;
                    p->finalHue   = static_cast<uint8_t>(newHue);
                }
            }
            else
            {
                if (p->isEnhancedHue)
                {
                    newHue = subtractEnhancedHue(p->initialEnhancedHue, p->finalEnhancedHue);
                    newHue = subtractEnhancedHue(p->finalEnhancedHue, newHue);

                    p->initialEnhancedHue = p->finalEnhancedHue;
                    p->finalEnhancedHue   = newHue;
                }
                else
                {
                    newHue = subtractHue(p->initialHue, p->finalHue);
                    newHue = subtractHue(p->finalHue, static_cast<uint8_t>(newHue));

                    p->initialHue = p->finalHue;
                    p->finalHue   = static_cast<uint8_t>(newHue);
                }
            }
        }

        p->stepsRemaining = p->stepsTotal;
    }

    return false;
}

/**
 * @brief Configures EventControl callback when using HSV colors
 *
 * @param endpoint
 */
EmberEventControl * ColorControlServer::configureHSVEventControl(EndpointId endpoint)
{
    EmberEventControl * controller = getEventControl(endpoint);
    VerifyOrReturnError(controller != nullptr, nullptr);

    controller->endpoint = endpoint;
    controller->callback = &emberAfPluginColorControlServerHueSatTransitionEventHandler;

    return controller;
}

/**
 * @brief Executes move to saturation command
 *
 * @param endpoint Target endpoint where to execute move
 * @param saturation Target saturation
 * @param transitionTime Transition time in 10th of seconds
 * @return Status::Success When successful,
 *         Status::UnsupportedEndpoint when the provided endpoint doesn't correspond with a saturation transition state,
 *         Status::ConstraintError if the saturation or tansitionTime are above maximum.
 */
Status ColorControlServer::moveToSaturation(EndpointId endpoint, uint8_t saturation, uint16_t transitionTime)
{
    VerifyOrReturnError(saturation <= MAX_SATURATION_VALUE, Status::ConstraintError);
    VerifyOrReturnError(transitionTime <= kMaxTransitionTime, Status::ConstraintError);
    Color16uTransitionState * colorSaturationTransitionState = getSaturationTransitionState(endpoint);
    VerifyOrReturnError(nullptr != colorSaturationTransitionState, Status::UnsupportedEndpoint);

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    // Handle color mode transition, if necessary.
    handleModeSwitch(endpoint, EnhancedColorMode::kCurrentHueAndCurrentSaturation);

    // now, kick off the state machine.
    initSaturationTransitionState(endpoint, colorSaturationTransitionState);
    colorSaturationTransitionState->finalValue     = saturation;
    colorSaturationTransitionState->stepsRemaining = std::max<uint16_t>(transitionTime, 1);
    colorSaturationTransitionState->stepsTotal     = colorSaturationTransitionState->stepsRemaining;
    colorSaturationTransitionState->timeRemaining  = transitionTime;
    colorSaturationTransitionState->transitionTime = transitionTime;
    colorSaturationTransitionState->endpoint       = endpoint;
    colorSaturationTransitionState->lowLimit       = MIN_SATURATION_VALUE;
    colorSaturationTransitionState->highLimit      = MAX_SATURATION_VALUE;

    SetHSVRemainingTime(endpoint);

    // kick off the state machine:
    scheduleTimerCallbackMs(configureHSVEventControl(endpoint), transitionTime ? TRANSITION_UPDATE_TIME_MS.count() : 0);

    return Status::Success;
}

/**
 * @brief Executes move to hue and saturatioan command.
 *
 * @param[in] endpoint EndpointId of the recipient Color control cluster.
 * @param[in] hue Target hue.
 * @param[in] saturation Target saturation.
 * @param[in] transitionTime Transition time in 10th of seconds.
 * @param[in] isEnhanced If True, function was called by EnhancedMoveHue command and rate is a uint16 value. If False function
 * was called by MoveHue command and rate is a uint8 value.
 * @return Status::Success When successful,
 *         Status::UnsupportedEndpoint when the provided endpoint doesn't correspond with a saturation transition state,
 *         Status::ConstraintError if the hue, saturation or transitionTime, are above maximum.
 */
Status ColorControlServer::moveToHueAndSaturation(EndpointId endpoint, uint16_t hue, uint8_t saturation, uint16_t transitionTime,
                                                  bool isEnhanced)
{
    uint16_t currentHue = 0;
    uint16_t halfWay    = isEnhanced ? HALF_MAX_UINT16T : HALF_MAX_UINT8T;
    bool moveUp;

    VerifyOrReturnError((isEnhanced || hue <= MAX_HUE_VALUE), Status::ConstraintError);
    VerifyOrReturnError(saturation <= MAX_SATURATION_VALUE, Status::ConstraintError);
    VerifyOrReturnError(transitionTime <= kMaxTransitionTime, Status::ConstraintError);

    uint16_t epIndex                                         = getEndpointIndex(endpoint);
    Color16uTransitionState * colorSaturationTransitionState = getSaturationTransitionStateByIndex(epIndex);
    ColorHueTransitionState * colorHueTransitionState        = getColorHueTransitionStateByIndex(epIndex);

    VerifyOrReturnError(nullptr != colorSaturationTransitionState, Status::UnsupportedEndpoint);
    VerifyOrReturnError(nullptr != colorHueTransitionState, Status::UnsupportedEndpoint);

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    // Handle color mode transition, if necessary.
    if (isEnhanced)
    {
        handleModeSwitch(endpoint, EnhancedColorMode::kEnhancedCurrentHueAndCurrentSaturation);
    }
    else
    {
        handleModeSwitch(endpoint, EnhancedColorMode::kCurrentHueAndCurrentSaturation);
    }

    // now, kick off the state machine.
    initHueTransitionState(endpoint, colorHueTransitionState, isEnhanced);

    if (isEnhanced)
    {
        currentHue                                = colorHueTransitionState->currentEnhancedHue;
        colorHueTransitionState->finalEnhancedHue = hue;
    }
    else
    {
        currentHue                        = static_cast<uint16_t>(colorHueTransitionState->currentHue);
        colorHueTransitionState->finalHue = static_cast<uint8_t>(hue);
    }

    // compute shortest direction
    if (hue > currentHue)
    {
        moveUp = (hue - currentHue) < halfWay;
    }
    else
    {
        moveUp = (currentHue - hue) > halfWay;
    }

    colorHueTransitionState->up             = moveUp;
    colorHueTransitionState->stepsRemaining = std::max<uint16_t>(transitionTime, 1);
    colorHueTransitionState->stepsTotal     = colorHueTransitionState->stepsRemaining;
    colorHueTransitionState->timeRemaining  = transitionTime;
    colorHueTransitionState->transitionTime = transitionTime;
    colorHueTransitionState->endpoint       = endpoint;
    colorHueTransitionState->repeat         = false;

    initSaturationTransitionState(endpoint, colorSaturationTransitionState);
    colorSaturationTransitionState->finalValue     = saturation;
    colorSaturationTransitionState->stepsRemaining = colorHueTransitionState->stepsRemaining;
    colorSaturationTransitionState->stepsTotal     = colorHueTransitionState->stepsRemaining;
    colorSaturationTransitionState->timeRemaining  = transitionTime;
    colorSaturationTransitionState->transitionTime = transitionTime;
    colorSaturationTransitionState->endpoint       = endpoint;
    colorSaturationTransitionState->lowLimit       = MIN_SATURATION_VALUE;
    colorSaturationTransitionState->highLimit      = MAX_SATURATION_VALUE;

    SetHSVRemainingTime(endpoint);

    // kick off the state machine:
    scheduleTimerCallbackMs(configureHSVEventControl(endpoint), transitionTime ? TRANSITION_UPDATE_TIME_MS.count() : 0);

    return Status::Success;
}

/**
 * @brief Executes move Hue Command.
 *
 * @param[in] endpoint EndpointId of the recipient Color control cluster.
 * @param[in] moveMode
 * @param[in] rate
 * @param[in] optionsMask
 * @param[in] optionsOverride
 * @param[in] isEnhanced If True, function was called by EnhancedMoveHue command and rate is a uint16 value. If False function
 * was called by MoveHue command and rate is a uint8 value.
 * @return Status::Success when successful,
 *         Status::InvalidCommand when Rate is 0 or an unknown moveMode is provided
 *         Status::UnsupportedEndpoint when the provided endpoint doesn't correspond with a hue transition state,
 */
Status ColorControlServer::moveHueCommand(EndpointId endpoint, HueMoveMode moveMode, uint16_t rate,
                                          BitMask<OptionsBitmap> optionsMask, BitMask<OptionsBitmap> optionsOverride,
                                          bool isEnhanced)
{
    MATTER_TRACE_SCOPE("moveHue", "ColorControl");

    // check moveMode and rate before any operation is done on the transition states
    // rate value is ignored if the MoveMode is stop
    VerifyOrReturnValue(moveMode != HueMoveMode::kUnknownEnumValue, Status::InvalidCommand);
    VerifyOrReturnValue((rate != 0 || moveMode == HueMoveMode::kStop), Status::InvalidCommand);

    uint16_t epIndex                                  = getEndpointIndex(endpoint);
    ColorHueTransitionState * colorHueTransitionState = getColorHueTransitionStateByIndex(epIndex);
    VerifyOrReturnValue(colorHueTransitionState != nullptr, Status::UnsupportedEndpoint);

    VerifyOrReturnValue(shouldExecuteIfOff(endpoint, optionsMask, optionsOverride), Status::Success);

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);
    // now, kick off the state machine.
    initHueTransitionState(endpoint, colorHueTransitionState, isEnhanced);

    if (moveMode == HueMoveMode::kStop)
    {
        // Per spec any saturation transition must also be cancelled.
        Color16uTransitionState * saturationState = getSaturationTransitionStateByIndex(epIndex);
        initSaturationTransitionState(endpoint, saturationState);
        return Status::Success;
    }

    // Handle color mode transition, if necessary.
    if (isEnhanced)
    {
        handleModeSwitch(endpoint, EnhancedColorMode::kEnhancedCurrentHueAndCurrentSaturation);
    }
    else
    {
        handleModeSwitch(endpoint, EnhancedColorMode::kCurrentHueAndCurrentSaturation);
    }

    if (moveMode == HueMoveMode::kUp)
    {
        if (isEnhanced)
        {
            colorHueTransitionState->finalEnhancedHue = addEnhancedHue(colorHueTransitionState->currentEnhancedHue, rate);
        }
        else
        {
            colorHueTransitionState->finalHue = addHue(colorHueTransitionState->currentHue, static_cast<uint8_t>(rate));
        }

        colorHueTransitionState->up = true;
    }
    else if (moveMode == HueMoveMode::kDown)
    {
        if (isEnhanced)
        {
            colorHueTransitionState->finalEnhancedHue = subtractEnhancedHue(colorHueTransitionState->currentEnhancedHue, rate);
        }
        else
        {
            colorHueTransitionState->finalHue = subtractHue(colorHueTransitionState->currentHue, static_cast<uint8_t>(rate));
        }

        colorHueTransitionState->up = false;
    }

    colorHueTransitionState->stepsRemaining = TRANSITION_STEPS_PER_1S;
    colorHueTransitionState->stepsTotal     = TRANSITION_STEPS_PER_1S;
    colorHueTransitionState->timeRemaining  = MAX_INT16U_VALUE;
    colorHueTransitionState->transitionTime = MAX_INT16U_VALUE;
    colorHueTransitionState->endpoint       = endpoint;
    colorHueTransitionState->repeat         = true;

    // hue movement can last forever. Indicate this with a remaining time of maxint
    SetQuietReportRemainingTime(endpoint, MAX_INT16U_VALUE, true /* isNewTransition */);

    // kick off the state machine:
    scheduleTimerCallbackMs(configureHSVEventControl(endpoint), TRANSITION_UPDATE_TIME_MS.count());
    return Status::Success;
}

/**
 * @brief Executes move to hue command.
 *
 * @param[in] endpoint EndpointId of the recipient Color control cluster.
 * @param[in] hue
 * @param[in] moveDirection
 * @param[in] transitionTime
 * @param[in] optionsMask
 * @param[in] optionsOverride
 * @param[in] isEnhanced If True, function was called by EnhancedMoveHue command and rate is a uint16 value. If False function
 * was called by MoveHue command and rate is a uint8 value.
 * @return Status::Success when successful,
 *         Status::InvalidCommand when Rate is 0 or an unknown moveDirection is provided
 *         Status::UnsupportedEndpoint when the provided endpoint doesn't correspond with a hue transition state,
 *         Status::ConstraintError when the other parameters are outside their defined value range.
 */
Status ColorControlServer::moveToHueCommand(EndpointId endpoint, uint16_t hue, DirectionEnum moveDirection, uint16_t transitionTime,
                                            BitMask<OptionsBitmap> optionsMask, BitMask<OptionsBitmap> optionsOverride,
                                            bool isEnhanced)
{
    MATTER_TRACE_SCOPE("moveToHue", "ColorControl");
    // Command Parameters constraint checks:
    VerifyOrReturnValue((isEnhanced || hue <= MAX_HUE_VALUE), Status::ConstraintError);
    VerifyOrReturnValue(transitionTime <= kMaxTransitionTime, Status::ConstraintError);

    VerifyOrReturnValue(moveDirection != DirectionEnum::kUnknownEnumValue, Status::InvalidCommand);

    ColorHueTransitionState * colorHueTransitionState = getColorHueTransitionState(endpoint);
    VerifyOrReturnValue(colorHueTransitionState != nullptr, Status::UnsupportedEndpoint);

    VerifyOrReturnValue(shouldExecuteIfOff(endpoint, optionsMask, optionsOverride), Status::Success);

    uint16_t currentHue = 0;
    DirectionEnum direction;
    if (isEnhanced)
    {
        Attributes::EnhancedCurrentHue::Get(endpoint, &currentHue);
    }
    else
    {
        uint8_t current8bitHue = 0;
        Attributes::CurrentHue::Get(endpoint, &current8bitHue);
        currentHue = static_cast<uint16_t>(current8bitHue);
    }

    // Convert the ShortestDistance/LongestDistance moveDirection values into Up/Down.
    switch (moveDirection)
    {
    case DirectionEnum::kShortest:
        if ((isEnhanced && (static_cast<uint16_t>(currentHue - hue) > HALF_MAX_UINT16T)) ||
            (!isEnhanced && (static_cast<uint8_t>(currentHue - hue) > HALF_MAX_UINT8T)))
        {
            direction = DirectionEnum::kUp;
        }
        else
        {
            direction = DirectionEnum::kDown;
        }
        break;
    case DirectionEnum::kLongest:
        if ((isEnhanced && (static_cast<uint16_t>(currentHue - hue) > HALF_MAX_UINT16T)) ||
            (!isEnhanced && (static_cast<uint8_t>(currentHue - hue) > HALF_MAX_UINT8T)))
        {
            direction = DirectionEnum::kDown;
        }
        else
        {
            direction = DirectionEnum::kUp;
        }
        break;
    case DirectionEnum::kUp:
    case DirectionEnum::kDown:
        direction = moveDirection;
        break;
    case DirectionEnum::kUnknownEnumValue:
        return Status::InvalidCommand;
        /* No default case, so if a new direction value gets added we will just fail
           to compile until we handle it correctly.  */
    }

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    // Handle color mode transition, if necessary.
    if (isEnhanced)
    {
        handleModeSwitch(endpoint, EnhancedColorMode::kEnhancedCurrentHueAndCurrentSaturation);
    }
    else
    {
        handleModeSwitch(endpoint, EnhancedColorMode::kCurrentHueAndCurrentSaturation);
    }

    // now, kick off the state machine.
    initHueTransitionState(endpoint, colorHueTransitionState, isEnhanced);

    if (isEnhanced)
    {
        colorHueTransitionState->finalEnhancedHue = hue;
    }
    else
    {
        colorHueTransitionState->finalHue = static_cast<uint8_t>(hue);
    }

    colorHueTransitionState->stepsRemaining = std::max<uint16_t>(transitionTime, 1);
    colorHueTransitionState->stepsTotal     = colorHueTransitionState->stepsRemaining;
    colorHueTransitionState->timeRemaining  = transitionTime;
    colorHueTransitionState->transitionTime = transitionTime;
    colorHueTransitionState->endpoint       = endpoint;
    colorHueTransitionState->up             = (direction == DirectionEnum::kUp);
    colorHueTransitionState->repeat         = false;

    SetHSVRemainingTime(endpoint);

    // kick off the state machine:
    scheduleTimerCallbackMs(configureHSVEventControl(endpoint), transitionTime ? TRANSITION_UPDATE_TIME_MS.count() : 0);
    return Status::Success;
}

/**
 * @brief Executes move to hue and saturatioan command.
 * @param[in] endpoint EndpointId of the recipient Color control cluster.
 * @param[in] hue
 * @param[in] saturation
 * @param[in] transitionTime
 * @param[in] optionsMask
 * @param[in] optionsOverride
 * @param[in] isEnhanced If True, function was called by EnhancedMoveHue command and rate is a uint16 value. If False function
 * was called by MoveHue command and rate is a uint8 value.
 * @return Status::Success when successful,
 *         Status::ConstraintError when the other parameters are outside their defined value range.
 */
Status ColorControlServer::moveToHueAndSaturationCommand(EndpointId endpoint, uint16_t hue, uint8_t saturation,
                                                         uint16_t transitionTime, BitMask<OptionsBitmap> optionsMask,
                                                         BitMask<OptionsBitmap> optionsOverride, bool isEnhanced)
{
    MATTER_TRACE_SCOPE("moveToHueAndSaturation", "ColorControl");
    // Command Parameters constraint checks:
    VerifyOrReturnValue((isEnhanced || hue <= MAX_HUE_VALUE), Status::ConstraintError);
    VerifyOrReturnValue(saturation <= MAX_SATURATION_VALUE, Status::ConstraintError);
    VerifyOrReturnValue(transitionTime <= kMaxTransitionTime, Status::ConstraintError);

    VerifyOrReturnValue(shouldExecuteIfOff(endpoint, optionsMask, optionsOverride), Status::Success);

    Status status = moveToHueAndSaturation(endpoint, hue, saturation, transitionTime, isEnhanced);
#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
    ScenesManagement::ScenesServer::Instance().MakeSceneInvalidForAllFabrics(endpoint);
#endif // MATTER_DM_PLUGIN_SCENES_MANAGEMENT
    return status;
}

/**
 * @brief Executes step hue command.
 *
 * @param[in] endpoint
 * @param[in] stepMode
 * @param[in] stepSize
 * @param[in] transitionTime
 * @param[in] optionsMask
 * @param[in] optionsOverride
 * @param[in] isEnhanced If True, function was called by EnhancedMoveHue command and rate is a uint16 value. If False function
 * was called by MoveHue command and rate is a uint8 value.
 * @return Status::Success when successful,
 *         Status::InvalidCommand when StepSize is 0 or an unknown HueStepMode is provided
 *         Status::UnsupportedEndpoint when the provided endpoint doesn't correspond with a hue transition state.
 *         Status::ConstraintError when the other parameters are outside their defined value range.
 */
Status ColorControlServer::stepHueCommand(EndpointId endpoint, HueStepMode stepMode, uint16_t stepSize, uint16_t transitionTime,
                                          BitMask<OptionsBitmap> optionsMask, BitMask<OptionsBitmap> optionsOverride,
                                          bool isEnhanced)
{
    MATTER_TRACE_SCOPE("stepHue", "ColorControl");
    // Command Parameters constraint checks:
    // The non-enhanced variant passed a uint8 type for transitionTime and the full range (0-255) is allowed
    if (isEnhanced)
    {
        VerifyOrReturnValue(transitionTime <= kMaxTransitionTime, Status::ConstraintError);
    }
    // Confirm validity of the step mode and step size received
    VerifyOrReturnValue(stepMode != HueStepMode::kUnknownEnumValue, Status::InvalidCommand);
    VerifyOrReturnValue(stepSize != 0, Status::InvalidCommand);

    ColorHueTransitionState * colorHueTransitionState = getColorHueTransitionState(endpoint);
    VerifyOrReturnValue(colorHueTransitionState != nullptr, Status::UnsupportedEndpoint);

    VerifyOrReturnValue(shouldExecuteIfOff(endpoint, optionsMask, optionsOverride), Status::Success);

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    // Handle color mode transition, if necessary.
    if (isEnhanced)
    {
        handleModeSwitch(endpoint, EnhancedColorMode::kEnhancedCurrentHueAndCurrentSaturation);
    }
    else
    {
        handleModeSwitch(endpoint, EnhancedColorMode::kCurrentHueAndCurrentSaturation);
    }

    // now, kick off the state machine.
    initHueTransitionState(endpoint, colorHueTransitionState, isEnhanced);

    if (isEnhanced)
    {

        if (stepMode == HueStepMode::kUp)
        {
            colorHueTransitionState->finalEnhancedHue = addEnhancedHue(colorHueTransitionState->currentEnhancedHue, stepSize);
            colorHueTransitionState->up               = true;
        }
        else if (stepMode == HueStepMode::kDown)
        {
            colorHueTransitionState->finalEnhancedHue = subtractEnhancedHue(colorHueTransitionState->currentEnhancedHue, stepSize);
            colorHueTransitionState->up               = false;
        }
    }
    else
    {
        if (stepMode == HueStepMode::kUp)
        {
            colorHueTransitionState->finalHue = addHue(colorHueTransitionState->currentHue, static_cast<uint8_t>(stepSize));
            colorHueTransitionState->up       = true;
        }
        else if (stepMode == HueStepMode::kDown)
        {
            colorHueTransitionState->finalHue = subtractHue(colorHueTransitionState->currentHue, static_cast<uint8_t>(stepSize));
            colorHueTransitionState->up       = false;
        }
    }

    colorHueTransitionState->stepsRemaining = std::max<uint16_t>(transitionTime, 1);
    colorHueTransitionState->stepsTotal     = colorHueTransitionState->stepsRemaining;
    colorHueTransitionState->timeRemaining  = transitionTime;
    colorHueTransitionState->transitionTime = transitionTime;
    colorHueTransitionState->endpoint       = endpoint;
    colorHueTransitionState->repeat         = false;

    SetHSVRemainingTime(endpoint);

    // kick off the state machine:
    scheduleTimerCallbackMs(configureHSVEventControl(endpoint), transitionTime ? TRANSITION_UPDATE_TIME_MS.count() : 0);

    return Status::Success;
}

/**
 * @brief Executes moveSaturation command.
 * @param endpoint EndpointId of the recipient Color control cluster.
 * @param commandData Struct containing the parameters of the command.
 * @return Status::Success when successful,
 *         Status::InvalidCommand when a rate of 0 for a non-stop move or an unknown SaturationMoveMode is provided
 *         Status::UnsupportedEndpoint when the provided endpoint doesn't correspond with a saturation transition state.
 */
Status ColorControlServer::moveSaturationCommand(EndpointId endpoint, const Commands::MoveSaturation::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("moveSaturation", "ColorControl");
    // check moveMode and rate before any operation is done on the transition states
    // rate value is ignored if the MoveMode is stop
    VerifyOrReturnValue(commandData.moveMode != SaturationMoveMode::kUnknownEnumValue, Status::InvalidCommand);
    VerifyOrReturnValue(commandData.rate != 0 || commandData.moveMode == SaturationMoveMode::kStop, Status::InvalidCommand);

    uint16_t epIndex                                         = getEndpointIndex(endpoint);
    Color16uTransitionState * colorSaturationTransitionState = getSaturationTransitionStateByIndex(epIndex);
    VerifyOrReturnValue(colorSaturationTransitionState != nullptr, Status::UnsupportedEndpoint);

    VerifyOrReturnValue(shouldExecuteIfOff(endpoint, commandData.optionsMask, commandData.optionsOverride), Status::Success);
    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);
    // now, kick off the state machine.
    initSaturationTransitionState(endpoint, colorSaturationTransitionState);
    if (commandData.moveMode == SaturationMoveMode::kStop)
    {
        // Per spec any hue transition must also be cancelled.
        ColorHueTransitionState * hueState = getColorHueTransitionStateByIndex(epIndex);
        initHueTransitionState(endpoint, hueState, false /*isEnhancedHue don't care*/);
        return Status::Success;
    }

    // Handle color mode transition, if necessary.
    handleModeSwitch(endpoint, EnhancedColorMode::kCurrentHueAndCurrentSaturation);

    if (commandData.moveMode == SaturationMoveMode::kUp)
    {
        colorSaturationTransitionState->finalValue = MAX_SATURATION_VALUE;
    }
    else if (commandData.moveMode == SaturationMoveMode::kDown)
    {
        colorSaturationTransitionState->finalValue = MIN_SATURATION_VALUE;
    }

    uint16_t transitionTime = computeTransitionTimeFromStateAndRate(colorSaturationTransitionState, commandData.rate);
    colorSaturationTransitionState->stepsRemaining = transitionTime;
    colorSaturationTransitionState->stepsTotal     = transitionTime;
    colorSaturationTransitionState->timeRemaining  = transitionTime;
    colorSaturationTransitionState->transitionTime = transitionTime;
    colorSaturationTransitionState->endpoint       = endpoint;
    colorSaturationTransitionState->lowLimit       = MIN_SATURATION_VALUE;
    colorSaturationTransitionState->highLimit      = MAX_SATURATION_VALUE;

    SetHSVRemainingTime(endpoint);

    // kick off the state machine:
    scheduleTimerCallbackMs(configureHSVEventControl(endpoint), TRANSITION_UPDATE_TIME_MS.count());
    return Status::Success;
}

/**
 * @brief Executes move to saturation command.
 * @param endpoint EndpointId of the recipient Color control cluster.
 * @param commandData Struct containing the parameters of the command.
 * @return Status::Success when successful,
 *         Status::UnsupportedEndpoint when the provided endpoint doesn't correspond with a saturation transition state (verified in
 * moveToSaturation function)
 *         Status::ConstraintError when a command parameter is outside its defined value range.
 */
Status ColorControlServer::moveToSaturationCommand(EndpointId endpoint,
                                                   const Commands::MoveToSaturation::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("moveToSaturation", "ColorControl");
    // Command Parameters constraint checks:
    VerifyOrReturnValue(commandData.saturation <= MAX_SATURATION_VALUE, Status::ConstraintError);
    VerifyOrReturnValue(commandData.transitionTime <= kMaxTransitionTime, Status::ConstraintError);

    VerifyOrReturnValue(shouldExecuteIfOff(endpoint, commandData.optionsMask, commandData.optionsOverride), Status::Success);
    Status status = moveToSaturation(endpoint, commandData.saturation, commandData.transitionTime);
#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
    ScenesManagement::ScenesServer::Instance().MakeSceneInvalidForAllFabrics(endpoint);
#endif // MATTER_DM_PLUGIN_SCENES_MANAGEMENT
    return status;
}

/**
 * @brief Executes step saturation command.
 * @param endpoint EndpointId of the recipient Color control cluster.
 * @param commandData Struct containing the parameters of the command.
 * @return Status::Success when successful,
 *         Status::InvalidCommand when a step size of 0 or an unknown SaturationStepMode is provided
 *         Status::UnsupportedEndpoint when the provided endpoint doesn't correspond with a saturation transition state,
 */
Status ColorControlServer::stepSaturationCommand(EndpointId endpoint, const Commands::StepSaturation::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("stepSaturation", "ColorControl");
    // Confirm validity of the step mode and step size received
    VerifyOrReturnValue(commandData.stepMode != SaturationStepMode::kUnknownEnumValue, Status::InvalidCommand);
    VerifyOrReturnValue(commandData.stepSize != 0, Status::InvalidCommand);

    Color16uTransitionState * colorSaturationTransitionState = getSaturationTransitionState(endpoint);
    VerifyOrReturnValue(colorSaturationTransitionState != nullptr, Status::UnsupportedEndpoint);

    VerifyOrReturnValue(shouldExecuteIfOff(endpoint, commandData.optionsMask, commandData.optionsOverride), Status::Success);

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);
    // Handle color mode transition, if necessary.
    handleModeSwitch(endpoint, EnhancedColorMode::kCurrentHueAndCurrentSaturation);
    // now, kick off the state machine.
    initSaturationTransitionState(endpoint, colorSaturationTransitionState);
    uint8_t currentSaturation = static_cast<uint8_t>(colorSaturationTransitionState->currentValue);

    if (commandData.stepMode == SaturationStepMode::kUp)
    {
        colorSaturationTransitionState->finalValue = addSaturation(currentSaturation, commandData.stepSize);
    }
    else if (commandData.stepMode == SaturationStepMode::kDown)
    {
        colorSaturationTransitionState->finalValue = subtractSaturation(currentSaturation, commandData.stepSize);
    }
    colorSaturationTransitionState->stepsRemaining = std::max<uint8_t>(commandData.transitionTime, 1);
    colorSaturationTransitionState->stepsTotal     = colorSaturationTransitionState->stepsRemaining;
    colorSaturationTransitionState->timeRemaining  = commandData.transitionTime;
    colorSaturationTransitionState->transitionTime = commandData.transitionTime;
    colorSaturationTransitionState->endpoint       = endpoint;
    colorSaturationTransitionState->lowLimit       = MIN_SATURATION_VALUE;
    colorSaturationTransitionState->highLimit      = MAX_SATURATION_VALUE;

    SetHSVRemainingTime(endpoint);

    // kick off the state machine:
    scheduleTimerCallbackMs(configureHSVEventControl(endpoint), commandData.transitionTime ? TRANSITION_UPDATE_TIME_MS.count() : 0);
    return Status::Success;
}

/**
 * @brief Executes ColorLoop command.
 * @param endpoint EndpointId of the recipient Color control cluster.
 * @param commandData Struct containing the parameters of the command.
 * @return Status::Success when successful,
 *         Status::InvalidCommand when an unknown action or direction is provided
 *         Status::UnsupportedEndpoint when the provided endpoint doesn't correspond with a hue transition state,
 */
Status ColorControlServer::colorLoopCommand(EndpointId endpoint, const Commands::ColorLoopSet::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("colorLoop", "ColorControl");
    // Validate the action and direction parameters of the command
    VerifyOrReturnValue(commandData.action != ColorLoopActionEnum::kUnknownEnumValue, Status::InvalidCommand);
    VerifyOrReturnValue(commandData.direction != ColorLoopDirectionEnum::kUnknownEnumValue, Status::InvalidCommand);

    uint16_t epIndex                                  = getEndpointIndex(endpoint);
    ColorHueTransitionState * colorHueTransitionState = getColorHueTransitionStateByIndex(epIndex);
    VerifyOrReturnValue(colorHueTransitionState != nullptr, Status::UnsupportedEndpoint);

    VerifyOrReturnValue(shouldExecuteIfOff(endpoint, commandData.optionsMask, commandData.optionsOverride), Status::Success);

    uint8_t isColorLoopActive = 0;
    // In case of get failure, isColorLoopActive will remain at the init value 0 (not active)
    if (Attributes::ColorLoopActive::Get(endpoint, &isColorLoopActive) != Status::Success)
    {
        ChipLogError(Zcl, "Failed to retrieve ColorLoopActive value");
    }

    uint8_t deactiveColorLoop =
        commandData.updateFlags.Has(ColorLoopUpdateFlags::kUpdateAction) && (commandData.action == ColorLoopAction::kDeactivate);

    if (commandData.updateFlags.Has(ColorLoopUpdateFlags::kUpdateDirection))
    {
        Attributes::ColorLoopDirection::Set(endpoint, to_underlying(commandData.direction));

        // Checks if color loop is active and stays active
        if (isColorLoopActive && !deactiveColorLoop)
        {
            colorHueTransitionState->up                 = (commandData.direction == ColorLoopDirectionEnum::kIncrement);
            colorHueTransitionState->initialEnhancedHue = colorHueTransitionState->currentEnhancedHue;

            if (commandData.direction == ColorLoopDirectionEnum::kIncrement)
            {
                colorHueTransitionState->finalEnhancedHue = static_cast<uint16_t>(colorHueTransitionState->initialEnhancedHue - 1);
            }
            else
            {
                colorHueTransitionState->finalEnhancedHue = static_cast<uint16_t>(colorHueTransitionState->initialEnhancedHue + 1);
            }
            colorHueTransitionState->stepsRemaining = colorHueTransitionState->stepsTotal;
        }
    }

    if (commandData.updateFlags.Has(ColorLoopUpdateFlags::kUpdateTime))
    {
        Attributes::ColorLoopTime::Set(endpoint, commandData.time);

        // Checks if color loop is active and stays active
        if (isColorLoopActive && !deactiveColorLoop)
        {
            colorHueTransitionState->stepsTotal         = static_cast<uint16_t>(commandData.time * TRANSITION_STEPS_PER_1S);
            colorHueTransitionState->initialEnhancedHue = colorHueTransitionState->currentEnhancedHue;

            if (colorHueTransitionState->up)
            {
                colorHueTransitionState->finalEnhancedHue = static_cast<uint16_t>(colorHueTransitionState->initialEnhancedHue - 1);
            }
            else
            {
                colorHueTransitionState->finalEnhancedHue = static_cast<uint16_t>(colorHueTransitionState->initialEnhancedHue + 1);
            }
            colorHueTransitionState->stepsRemaining = colorHueTransitionState->stepsTotal;
        }
    }

    if (commandData.updateFlags.Has(ColorLoopUpdateFlags::kUpdateStartHue))
    {
        Attributes::ColorLoopStartEnhancedHue::Set(endpoint, commandData.startHue);
    }

    if (commandData.updateFlags.Has(ColorLoopUpdateFlags::kUpdateAction))
    {
        if (commandData.action == ColorLoopAction::kDeactivate)
        {
            if (isColorLoopActive)
            {
                stopAllColorTransitions(endpoint);

                Attributes::ColorLoopActive::Set(endpoint, false);

                uint16_t storedEnhancedHue = 0;
                Attributes::ColorLoopStoredEnhancedHue::Get(endpoint, &storedEnhancedHue);
                MarkAttributeDirty markDirty =
                    SetQuietReportAttribute(quietEnhancedHue[epIndex], storedEnhancedHue, true /*isEndOfTransition*/, 0);
                Attributes::EnhancedCurrentHue::Set(endpoint, quietEnhancedHue[epIndex].value().Value(), markDirty);
            }
            else
            {
                // Do Nothing since it's not on
            }
        }
        else if (commandData.action == ColorLoopAction::kActivateFromColorLoopStartEnhancedHue)
        {
            startColorLoop(endpoint, true);
        }
        else if (commandData.action == ColorLoopAction::kActivateFromEnhancedCurrentHue)
        {
            startColorLoop(endpoint, false);
        }
    }

#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
    ScenesManagement::ScenesServer::Instance().MakeSceneInvalidForAllFabrics(endpoint);
#endif // MATTER_DM_PLUGIN_SCENES_MANAGEMENT
    return Status::Success;
}

/**
 * @brief Updates Hue and saturation after timer is finished.
 *
 * @param endpoint
 */
void ColorControlServer::updateHueSatCommand(EndpointId endpoint)
{
    MATTER_TRACE_SCOPE("updateHueSat", "ColorControl");
    uint16_t epIndex                                         = getEndpointIndex(endpoint);
    ColorHueTransitionState * colorHueTransitionState        = getColorHueTransitionStateByIndex(epIndex);
    Color16uTransitionState * colorSaturationTransitionState = getSaturationTransitionStateByIndex(epIndex);

    uint8_t previousHue          = colorHueTransitionState->currentHue;
    uint16_t previousSaturation  = colorSaturationTransitionState->currentValue;
    uint16_t previousEnhancedhue = colorHueTransitionState->currentEnhancedHue;

    bool isHueTansitionDone         = computeNewHueValue(colorHueTransitionState);
    bool isSaturationTransitionDone = computeNewColor16uValue(colorSaturationTransitionState);

    SetHSVRemainingTime(endpoint);

    if (isHueTansitionDone && isSaturationTransitionDone)
    {
        stopAllColorTransitions(endpoint);
    }
    else
    {
        scheduleTimerCallbackMs(configureHSVEventControl(endpoint), TRANSITION_UPDATE_TIME_MS.count());
    }

    uint8_t currentHue;
    MarkAttributeDirty markDirty;
    if (colorHueTransitionState->isEnhancedHue)
    {
        markDirty = SetQuietReportAttribute(quietEnhancedHue[epIndex], colorHueTransitionState->currentEnhancedHue,
                                            isHueTansitionDone, colorHueTransitionState->transitionTime);
        Attributes::EnhancedCurrentHue::Set(endpoint, quietEnhancedHue[epIndex].value().Value(), markDirty);
        currentHue = static_cast<uint8_t>(colorHueTransitionState->currentEnhancedHue >> 8);

        if (previousEnhancedhue != colorHueTransitionState->currentEnhancedHue)
        {
            ChipLogProgress(Zcl, "Enhanced Hue %d endpoint %d", colorHueTransitionState->currentEnhancedHue, endpoint);
        }
    }
    else
    {
        currentHue = colorHueTransitionState->currentHue;
        if (previousHue != colorHueTransitionState->currentHue)
        {
            ChipLogProgress(Zcl, "Hue %d endpoint %d", colorHueTransitionState->currentHue, endpoint);
        }
    }

    markDirty = SetQuietReportAttribute(quietHue[epIndex], currentHue, isHueTansitionDone, colorHueTransitionState->transitionTime);
    Attributes::CurrentHue::Set(endpoint, quietHue[epIndex].value().Value(), markDirty);

    if (previousSaturation != colorSaturationTransitionState->currentValue)
    {
        ChipLogProgress(Zcl, "Saturation %d endpoint %d", colorSaturationTransitionState->currentValue, endpoint);
    }

    markDirty = SetQuietReportAttribute(quietSaturation[epIndex], colorSaturationTransitionState->currentValue,
                                        isSaturationTransitionDone, colorSaturationTransitionState->transitionTime);
    Attributes::CurrentSaturation::Set(endpoint, quietSaturation[epIndex].value().Value(), markDirty);

    computePwmFromHsv(endpoint);
}

#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_XY

/**
 * @brief Returns Color16uTransitionState for X color associated to an endpoint index
 *
 * @param endpoint
 * @return ColorControlServer::Color16uTransitionState*
 */
ColorControlServer::Color16uTransitionState * ColorControlServer::getXTransitionStateByIndex(uint16_t index)
{
    Color16uTransitionState * state = nullptr;
    if (index < MATTER_ARRAY_SIZE(colorXtransitionStates))
    {
        state = &colorXtransitionStates[index];
    }

    return state;
}

/**
 * @brief Returns Color16uTransitionState for X color associated to an endpoint
 *
 * @param endpoint
 * @return ColorControlServer::Color16uTransitionState*
 */
ColorControlServer::Color16uTransitionState * ColorControlServer::getXTransitionState(EndpointId endpoint)
{
    return getXTransitionStateByIndex(getEndpointIndex(endpoint));
}

/**
 * @brief Returns Color16uTransitionState for Y color associated to an endpoint index
 *
 * @param endpoint
 * @return ColorControlServer::Color16uTransitionState*
 */
ColorControlServer::Color16uTransitionState * ColorControlServer::getYTransitionStateByIndex(uint16_t index)
{
    Color16uTransitionState * state = nullptr;
    if (index < MATTER_ARRAY_SIZE(colorYtransitionStates))
    {
        state = &colorYtransitionStates[index];
    }

    return state;
}

/**
 * @brief Returns Color16uTransitionState for Y color associated to an endpoint
 *
 * @param endpoint
 * @return ColorControlServer::Color16uTransitionState*
 */
ColorControlServer::Color16uTransitionState * ColorControlServer::getYTransitionState(EndpointId endpoint)
{
    return getYTransitionStateByIndex(getEndpointIndex(endpoint));
}

uint16_t ColorControlServer::findNewColorValueFromStep(uint16_t oldValue, int16_t step)
{
    uint16_t newValue;
    int32_t newValueSigned;

    newValueSigned = ((int32_t) oldValue) + ((int32_t) step);

    if (newValueSigned < 0)
    {
        newValue = 0;
    }
    else if (newValueSigned > MAX_CIE_XY_VALUE)
    {
        newValue = MAX_CIE_XY_VALUE;
    }
    else
    {
        newValue = (uint16_t) newValueSigned;
    }

    return newValue;
}

/**
 * @brief Configures EventControl callback when using XY colors
 *
 * @param endpoint
 */
EmberEventControl * ColorControlServer::configureXYEventControl(EndpointId endpoint)
{
    EmberEventControl * controller = getEventControl(endpoint);
    VerifyOrReturnError(controller != nullptr, nullptr);

    controller->endpoint = endpoint;
    controller->callback = &emberAfPluginColorControlServerXyTransitionEventHandler;

    return controller;
}

/**
 * @brief executes move to saturation command
 *
 * @param endpoint target endpoint where to execute move
 * @param colorX target X
 * @param colorY target Y
 * @param transitionTime transition time in 10th of seconds
 * @return Status::Success if successful,
 * @return Status::Success when successful,
 *         Status::UnsupportedEndpoint XY is not supported on the endpoint,
 *         Status::ConstraintError when a command parameter is outside its defined value range.
 */
Status ColorControlServer::moveToColor(EndpointId endpoint, uint16_t colorX, uint16_t colorY, uint16_t transitionTime)
{
    // Command Parameters constraint checks:
    VerifyOrReturnValue(colorX <= MAX_CIE_XY_VALUE, Status::ConstraintError);
    VerifyOrReturnValue(colorY <= MAX_CIE_XY_VALUE, Status::ConstraintError);
    VerifyOrReturnValue(transitionTime <= kMaxTransitionTime, Status::ConstraintError);

    uint16_t epIndex                                = getEndpointIndex(endpoint);
    Color16uTransitionState * colorXTransitionState = getXTransitionStateByIndex(epIndex);
    Color16uTransitionState * colorYTransitionState = getYTransitionStateByIndex(epIndex);

    VerifyOrReturnError(nullptr != colorXTransitionState, Status::UnsupportedEndpoint);
    VerifyOrReturnError(nullptr != colorYTransitionState, Status::UnsupportedEndpoint);

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    // Handle color mode transition, if necessary.
    handleModeSwitch(endpoint, EnhancedColorMode::kCurrentXAndCurrentY);

    // now, kick off the state machine.
    Attributes::CurrentX::Get(endpoint, &(colorXTransitionState->initialValue));
    Attributes::CurrentX::Get(endpoint, &(colorXTransitionState->currentValue));
    colorXTransitionState->finalValue     = colorX;
    colorXTransitionState->stepsRemaining = std::max<uint16_t>(transitionTime, 1);
    colorXTransitionState->stepsTotal     = colorXTransitionState->stepsRemaining;
    colorXTransitionState->timeRemaining  = transitionTime;
    colorXTransitionState->transitionTime = transitionTime;
    colorXTransitionState->endpoint       = endpoint;
    colorXTransitionState->lowLimit       = MIN_CIE_XY_VALUE;
    colorXTransitionState->highLimit      = MAX_CIE_XY_VALUE;

    Attributes::CurrentY::Get(endpoint, &(colorYTransitionState->initialValue));
    Attributes::CurrentY::Get(endpoint, &(colorYTransitionState->currentValue));
    colorYTransitionState->finalValue     = colorY;
    colorYTransitionState->stepsRemaining = colorXTransitionState->stepsRemaining;
    colorYTransitionState->stepsTotal     = colorXTransitionState->stepsRemaining;
    colorYTransitionState->timeRemaining  = transitionTime;
    colorYTransitionState->transitionTime = transitionTime;
    colorYTransitionState->endpoint       = endpoint;
    colorYTransitionState->lowLimit       = MIN_CIE_XY_VALUE;
    colorYTransitionState->highLimit      = MAX_CIE_XY_VALUE;

    SetQuietReportRemainingTime(endpoint, transitionTime, true /* isNewTransition */);

    // kick off the state machine:
    scheduleTimerCallbackMs(configureXYEventControl(endpoint), transitionTime ? TRANSITION_UPDATE_TIME_MS.count() : 0);

    return Status::Success;
}

/**
 * @brief executes move to color command
 * @param endpoint endpointId of the recipient Color control cluster
 * @param commandData Struct containing the parameters of the command
 * @return Status::Success when successful,
 *         Status::UnsupportedEndpoint when the provided endpoint doesn't correspond with a Color XY transition state (verified in
 * moveToColor function),
 *         Status::ConstraintError when a command parameter is outside its defined value range.
 */
Status ColorControlServer::moveToColorCommand(EndpointId endpoint, const Commands::MoveToColor::DecodableType & commandData)
{
    // Command Parameters constraint checks:
    VerifyOrReturnValue(commandData.colorX <= MAX_CIE_XY_VALUE, Status::ConstraintError);
    VerifyOrReturnValue(commandData.colorY <= MAX_CIE_XY_VALUE, Status::ConstraintError);
    VerifyOrReturnValue(commandData.transitionTime <= kMaxTransitionTime, Status::ConstraintError);

    VerifyOrReturnValue(shouldExecuteIfOff(endpoint, commandData.optionsMask, commandData.optionsOverride), Status::Success);

    Status status = moveToColor(endpoint, commandData.colorX, commandData.colorY, commandData.transitionTime);
#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
    ScenesManagement::ScenesServer::Instance().MakeSceneInvalidForAllFabrics(endpoint);
#endif // MATTER_DM_PLUGIN_SCENES_MANAGEMENT
    return status;
}

/**
 * @brief executes move color command
 * @param endpoint endpointId of the recipient Color control cluster
 * @param commandData Struct containing the parameters of the command
 * @return Status::Success when successful,
 *         Status::UnsupportedEndpoint when the provided endpoint doesn't correspond with a Color XY transition state (verified in
 * moveToColor).
 */
Status ColorControlServer::moveColorCommand(EndpointId endpoint, const Commands::MoveColor::DecodableType & commandData)
{
    uint16_t epIndex                                = getEndpointIndex(endpoint);
    Color16uTransitionState * colorXTransitionState = getXTransitionStateByIndex(epIndex);
    Color16uTransitionState * colorYTransitionState = getYTransitionStateByIndex(epIndex);
    VerifyOrReturnValue(colorXTransitionState != nullptr, Status::UnsupportedEndpoint);
    VerifyOrReturnValue(colorYTransitionState != nullptr, Status::UnsupportedEndpoint);

    VerifyOrReturnValue(shouldExecuteIfOff(endpoint, commandData.optionsMask, commandData.optionsOverride), Status::Success);

    uint16_t transitionTimeX, transitionTimeY;
    uint16_t unsignedRate;

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    if (commandData.rateX == 0 && commandData.rateY == 0)
    {
        // any current transition has been stopped. We are done.
        return Status::Success;
    }

    // Handle color mode transition, if necessary.
    handleModeSwitch(endpoint, EnhancedColorMode::kCurrentXAndCurrentY);

    // now, kick off the state machine.
    Attributes::CurrentX::Get(endpoint, &(colorXTransitionState->initialValue));
    colorXTransitionState->currentValue = colorXTransitionState->initialValue;
    if (commandData.rateX > 0)
    {
        colorXTransitionState->finalValue = MAX_CIE_XY_VALUE;
        unsignedRate                      = static_cast<uint16_t>(commandData.rateX);
    }
    else
    {
        colorXTransitionState->finalValue = MIN_CIE_XY_VALUE;
        unsignedRate                      = static_cast<uint16_t>(commandData.rateX * -1);
    }
    transitionTimeX                       = computeTransitionTimeFromStateAndRate(colorXTransitionState, unsignedRate);
    colorXTransitionState->stepsRemaining = transitionTimeX;
    colorXTransitionState->stepsTotal     = transitionTimeX;
    colorXTransitionState->timeRemaining  = transitionTimeX;
    colorXTransitionState->transitionTime = transitionTimeX;
    colorXTransitionState->endpoint       = endpoint;
    colorXTransitionState->lowLimit       = MIN_CIE_XY_VALUE;
    colorXTransitionState->highLimit      = MAX_CIE_XY_VALUE;

    Attributes::CurrentY::Get(endpoint, &(colorYTransitionState->initialValue));
    colorYTransitionState->currentValue = colorYTransitionState->initialValue;
    if (commandData.rateY > 0)
    {
        colorYTransitionState->finalValue = MAX_CIE_XY_VALUE;
        unsignedRate                      = static_cast<uint16_t>(commandData.rateY);
    }
    else
    {
        colorYTransitionState->finalValue = MIN_CIE_XY_VALUE;
        unsignedRate                      = static_cast<uint16_t>(commandData.rateY * -1);
    }
    transitionTimeY                       = computeTransitionTimeFromStateAndRate(colorYTransitionState, unsignedRate);
    colorYTransitionState->stepsRemaining = transitionTimeY;
    colorYTransitionState->stepsTotal     = transitionTimeY;
    colorYTransitionState->timeRemaining  = transitionTimeY;
    colorYTransitionState->transitionTime = transitionTimeY;
    colorYTransitionState->endpoint       = endpoint;
    colorYTransitionState->lowLimit       = MIN_CIE_XY_VALUE;
    colorYTransitionState->highLimit      = MAX_CIE_XY_VALUE;

    SetQuietReportRemainingTime(endpoint, std::max(transitionTimeX, transitionTimeY), true /* isNewTransition */);

    // kick off the state machine:
    scheduleTimerCallbackMs(configureXYEventControl(endpoint), TRANSITION_UPDATE_TIME_MS.count());
    return Status::Success;
}

/**
 * @brief executes step color command
 * @param endpoint endpointId of the recipient Color control cluster
 * @param commandData Struct containing the parameters of the command
 * @return Status::Success when successful,
 *         Status::InvalidCommand when a step X and Y of 0 is provided
 *         Status::UnsupportedEndpoint when the provided endpoint doesn't correspond with a Color XY transition state,
 *         Status::ConstraintError when a command parameter is outside its defined value range.
 */
Status ColorControlServer::stepColorCommand(EndpointId endpoint, const Commands::StepColor::DecodableType & commandData)
{
    // Command Parameters constraint checks:
    VerifyOrReturnValue(commandData.transitionTime <= kMaxTransitionTime, Status::ConstraintError);

    VerifyOrReturnValue(commandData.stepX != 0 || commandData.stepY != 0, Status::InvalidCommand);

    uint16_t epIndex                                = getEndpointIndex(endpoint);
    Color16uTransitionState * colorXTransitionState = getXTransitionStateByIndex(epIndex);
    Color16uTransitionState * colorYTransitionState = getYTransitionStateByIndex(epIndex);

    VerifyOrReturnValue(colorXTransitionState != nullptr, Status::UnsupportedEndpoint);
    VerifyOrReturnValue(colorYTransitionState != nullptr, Status::UnsupportedEndpoint);
    VerifyOrReturnValue(shouldExecuteIfOff(endpoint, commandData.optionsMask, commandData.optionsOverride), Status::Success);

    uint16_t currentColorX = 0;
    uint16_t currentColorY = 0;
    Attributes::CurrentX::Get(endpoint, &currentColorX);
    Attributes::CurrentY::Get(endpoint, &currentColorY);

    uint16_t colorX = findNewColorValueFromStep(currentColorX, commandData.stepX);
    uint16_t colorY = findNewColorValueFromStep(currentColorY, commandData.stepY);

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    // Handle color mode transition, if necessary.
    handleModeSwitch(endpoint, EnhancedColorMode::kCurrentXAndCurrentY);

    // now, kick off the state machine.
    colorXTransitionState->initialValue   = currentColorX;
    colorXTransitionState->currentValue   = currentColorX;
    colorXTransitionState->finalValue     = colorX;
    colorXTransitionState->stepsRemaining = std::max<uint16_t>(commandData.transitionTime, 1);
    colorXTransitionState->stepsTotal     = colorXTransitionState->stepsRemaining;
    colorXTransitionState->timeRemaining  = commandData.transitionTime;
    colorXTransitionState->transitionTime = commandData.transitionTime;
    colorXTransitionState->endpoint       = endpoint;
    colorXTransitionState->lowLimit       = MIN_CIE_XY_VALUE;
    colorXTransitionState->highLimit      = MAX_CIE_XY_VALUE;

    colorYTransitionState->initialValue   = currentColorY;
    colorYTransitionState->currentValue   = currentColorY;
    colorYTransitionState->finalValue     = colorY;
    colorYTransitionState->stepsRemaining = colorXTransitionState->stepsRemaining;
    colorYTransitionState->stepsTotal     = colorXTransitionState->stepsRemaining;
    colorYTransitionState->timeRemaining  = commandData.transitionTime;
    colorYTransitionState->transitionTime = commandData.transitionTime;
    colorYTransitionState->endpoint       = endpoint;
    colorYTransitionState->lowLimit       = MIN_CIE_XY_VALUE;
    colorYTransitionState->highLimit      = MAX_CIE_XY_VALUE;

    SetQuietReportRemainingTime(endpoint, commandData.transitionTime, true /* isNewTransition */);

    // kick off the state machine:
    scheduleTimerCallbackMs(configureXYEventControl(endpoint), commandData.transitionTime ? TRANSITION_UPDATE_TIME_MS.count() : 0);
    return Status::Success;
}

/**
 * @brief Update XY color after timer is finished
 *
 * @param endpoint
 */
void ColorControlServer::updateXYCommand(EndpointId endpoint)
{
    uint16_t epIndex                                = getEndpointIndex(endpoint);
    Color16uTransitionState * colorXTransitionState = getXTransitionStateByIndex(epIndex);
    Color16uTransitionState * colorYTransitionState = getYTransitionStateByIndex(epIndex);

    // compute new values for X and Y.
    bool isXTransitionDone = computeNewColor16uValue(colorXTransitionState);
    bool isYTransitionDone = computeNewColor16uValue(colorYTransitionState);

    SetQuietReportRemainingTime(endpoint, std::max(colorXTransitionState->timeRemaining, colorYTransitionState->timeRemaining));

    if (isXTransitionDone && isYTransitionDone)
    {
        stopAllColorTransitions(endpoint);
    }
    else
    {
        scheduleTimerCallbackMs(configureXYEventControl(endpoint), TRANSITION_UPDATE_TIME_MS.count());
    }

    MarkAttributeDirty markXDirty = SetQuietReportAttribute(quietColorX[epIndex], colorXTransitionState->currentValue,
                                                            isXTransitionDone, colorXTransitionState->transitionTime);
    MarkAttributeDirty markYDirty = SetQuietReportAttribute(quietColorY[epIndex], colorYTransitionState->currentValue,
                                                            isYTransitionDone, colorYTransitionState->transitionTime);

    Attributes::CurrentX::Set(endpoint, quietColorX[epIndex].value().Value(), markXDirty);
    Attributes::CurrentY::Set(endpoint, quietColorY[epIndex].value().Value(), markYDirty);

    ChipLogProgress(Zcl, "Color X %d Color Y %d", colorXTransitionState->currentValue, colorYTransitionState->currentValue);

    computePwmFromXy(endpoint);
}

#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_XY

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP
/**
 * @brief Get the Temp Transition State object associated to the endpoint index
 *
 * @param endpoint
 * @return Color16uTransitionState*
 */
ColorControlServer::Color16uTransitionState * ColorControlServer::getTempTransitionStateByIndex(uint16_t index)
{
    Color16uTransitionState * state = nullptr;
    if (index < MATTER_ARRAY_SIZE(colorTempTransitionStates))
    {
        state = &colorTempTransitionStates[index];
    }

    return state;
}

/**
 * @brief Get the Temp Transition State object associated to the endpoint
 *
 * @param endpoint
 * @return Color16uTransitionState*
 */
ColorControlServer::Color16uTransitionState * ColorControlServer::getTempTransitionState(EndpointId endpoint)
{
    return getTempTransitionStateByIndex(getEndpointIndex(endpoint));
}

/**
 * @brief Executes move to color temp logic.
 *
 * @param aEndpoint
 * @param colorTemperature
 * @param transitionTime
 * @return Status::Success if successful, Status::UnsupportedEndpoint if the endpoint doesn't support color temperature.
 */
Status ColorControlServer::moveToColorTemp(EndpointId aEndpoint, uint16_t colorTemperature, uint16_t transitionTime)
{
    // Command Parameters constraint checks:
    VerifyOrReturnValue(colorTemperature <= kMaxColorTemperatureMireds, Status::ConstraintError);
    VerifyOrReturnValue(transitionTime <= kMaxTransitionTime, Status::ConstraintError);

    EndpointId endpoint                                = aEndpoint;
    Color16uTransitionState * colorTempTransitionState = getTempTransitionState(endpoint);
    VerifyOrReturnError(nullptr != colorTempTransitionState, Status::UnsupportedEndpoint);

    uint16_t temperatureMin = MIN_TEMPERATURE_VALUE;
    Attributes::ColorTempPhysicalMinMireds::Get(endpoint, &temperatureMin);

    // Avoid potential divide-by-zero in future Kelvin conversions.
    temperatureMin = std::max(static_cast<uint16_t>(1u), temperatureMin);

    uint16_t temperatureMax = MAX_TEMPERATURE_VALUE;
    Attributes::ColorTempPhysicalMaxMireds::Get(endpoint, &temperatureMax);

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    // Handle color mode transition, if necessary.
    handleModeSwitch(endpoint, EnhancedColorMode::kColorTemperatureMireds);

    if (colorTemperature < temperatureMin)
    {
        colorTemperature = temperatureMin;
    }

    if (colorTemperature > temperatureMax)
    {
        colorTemperature = temperatureMax;
    }

    // now, kick off the state machine.
    Attributes::ColorTemperatureMireds::Get(endpoint, &(colorTempTransitionState->initialValue));
    Attributes::ColorTemperatureMireds::Get(endpoint, &(colorTempTransitionState->currentValue));

    colorTempTransitionState->finalValue     = colorTemperature;
    colorTempTransitionState->stepsRemaining = std::max<uint16_t>(transitionTime, 1);
    colorTempTransitionState->stepsTotal     = colorTempTransitionState->stepsRemaining;
    colorTempTransitionState->timeRemaining  = transitionTime;
    colorTempTransitionState->transitionTime = transitionTime;
    colorTempTransitionState->endpoint       = endpoint;
    colorTempTransitionState->lowLimit       = temperatureMin;
    colorTempTransitionState->highLimit      = temperatureMax;

    SetQuietReportRemainingTime(endpoint, transitionTime, true /* isNewTransition */);

    // kick off the state machine
    scheduleTimerCallbackMs(configureTempEventControl(endpoint), transitionTime ? TRANSITION_UPDATE_TIME_MS.count() : 0);
    return Status::Success;
}

/**
 * @brief returns Temperature coupled to level minimum
 *
 * @param endpoint
 * @return uint16_t
 */
uint16_t ColorControlServer::getTemperatureCoupleToLevelMin(EndpointId endpoint)
{
    uint16_t colorTemperatureCoupleToLevelMin;
    Status status;

    status = Attributes::CoupleColorTempToLevelMinMireds::Get(endpoint, &colorTemperatureCoupleToLevelMin);

    if (status != Status::Success)
    {
        // Not less than the physical min.
        Attributes::ColorTempPhysicalMinMireds::Get(endpoint, &colorTemperatureCoupleToLevelMin);

        // Avoid potential divide-by-zero in future Kelvin conversions.
        colorTemperatureCoupleToLevelMin = std::max(static_cast<uint16_t>(1u), colorTemperatureCoupleToLevelMin);
    }

    return colorTemperatureCoupleToLevelMin;
}

/**
 * @brief Configures EventControl callback when using Temp colors
 *
 * @param endpoint
 */
EmberEventControl * ColorControlServer::configureTempEventControl(EndpointId endpoint)
{
    EmberEventControl * controller = getEventControl(endpoint);
    VerifyOrReturnError(controller != nullptr, nullptr);

    controller->endpoint = endpoint;
    controller->callback = &emberAfPluginColorControlServerTempTransitionEventHandler;

    return controller;
}

void ColorControlServer::startUpColorTempCommand(EndpointId endpoint)
{
    // 07-5123-07 (i.e. ZCL 7) 5.2.2.2.1.22 StartUpColorTemperatureMireds Attribute
    // The StartUpColorTemperatureMireds attribute SHALL define the desired startup color
    // temperature values a lamp SHALL use when it is supplied with power and this value SHALL
    // be reflected in the ColorTemperatureMireds attribute. In addition, the ColorMode and
    // EnhancedColorMode attributes SHALL be set to 0x02 (color temperature). The values of
    // the StartUpColorTemperatureMireds attribute are listed in the table below.
    // Value                Action on power up
    // 0x0001-0xffef        Set the ColorTemperatureMireds attribute to this value.
    // null                 Set the ColorTemperatureMireds attribute to its previous value.

    // Initialize startUpColorTempMireds to "maintain previous value" value null
    app::DataModel::Nullable<uint16_t> startUpColorTemp = DataModel::NullNullable;
    Status status = Attributes::StartUpColorTemperatureMireds::Get(endpoint, startUpColorTemp);

    if (status == Status::Success && !startUpColorTemp.IsNull())
    {
        uint16_t tempPhysicalMin = MIN_TEMPERATURE_VALUE;
        Attributes::ColorTempPhysicalMinMireds::Get(endpoint, &tempPhysicalMin);
        // Avoid potential divide-by-zero in future Kelvin conversions.
        tempPhysicalMin = std::max(static_cast<uint16_t>(1u), tempPhysicalMin);

        uint16_t tempPhysicalMax = MAX_TEMPERATURE_VALUE;
        Attributes::ColorTempPhysicalMaxMireds::Get(endpoint, &tempPhysicalMax);

        if (tempPhysicalMin <= startUpColorTemp.Value() && startUpColorTemp.Value() <= tempPhysicalMax)
        {
            // Apply valid startup color temp value that is within physical limits of device.
            // Otherwise, the startup value is outside the device's supported range, and the
            // existing setting of ColorTemp attribute will be left unchanged (i.e., treated as
            // if startup color temp was set to null).
            uint16_t epIndex             = getEndpointIndex(endpoint);
            MarkAttributeDirty markDirty = SetQuietReportAttribute(quietTemperatureMireds[epIndex], startUpColorTemp.Value(),
                                                                   false /* isEndOfTransition */, 0);
            status = Attributes::ColorTemperatureMireds::Set(endpoint, quietTemperatureMireds[epIndex].value().Value(), markDirty);

            if (status == Status::Success)
            {
                // Set ColorMode attributes to reflect ColorTemperature.
                auto updateColorMode = ColorModeEnum::kColorTemperatureMireds;
                Attributes::ColorMode::Set(endpoint, updateColorMode);

                Attributes::EnhancedColorMode::Set(endpoint, static_cast<EnhancedColorModeEnum>(updateColorMode));
            }
        }
    }
}

/**
 * @brief updates color temp when timer is finished
 *
 * @param endpoint
 */
void ColorControlServer::updateTempCommand(EndpointId endpoint)
{
    uint16_t epIndex                                   = getEndpointIndex(endpoint);
    Color16uTransitionState * colorTempTransitionState = getTempTransitionStateByIndex(epIndex);
    bool isColorTempTransitionDone;

    isColorTempTransitionDone = computeNewColor16uValue(colorTempTransitionState);

    if (!isColorTempTransitionDone)
    {
        // Check whether our color temperature has actually changed.  If not, do
        // nothing, and wait for it to change.
        uint16_t currentColorTemp;
        if (Attributes::ColorTemperatureMireds::Get(endpoint, &currentColorTemp) != Status::Success)
        {
            // Why can't we read our attribute?
            return;
        }

        if (currentColorTemp == colorTempTransitionState->currentValue)
        {
            scheduleTimerCallbackMs(configureTempEventControl(endpoint), TRANSITION_UPDATE_TIME_MS.count());
            return;
        }
    }

    SetQuietReportRemainingTime(endpoint, colorTempTransitionState->timeRemaining);

    if (isColorTempTransitionDone)
    {
        stopAllColorTransitions(endpoint);
    }
    else
    {
        scheduleTimerCallbackMs(configureTempEventControl(endpoint), TRANSITION_UPDATE_TIME_MS.count());
    }

    MarkAttributeDirty markDirty = SetQuietReportAttribute(quietTemperatureMireds[epIndex], colorTempTransitionState->currentValue,
                                                           isColorTempTransitionDone, colorTempTransitionState->timeRemaining);
    Attributes::ColorTemperatureMireds::Set(endpoint, quietTemperatureMireds[epIndex].value().Value(), markDirty);

    ChipLogProgress(Zcl, "Color Temperature %d", colorTempTransitionState->currentValue);

    computePwmFromTemp(endpoint);
}

/**
 * @brief Executes move color temp command.
 * @param endpoint EndpointId of the recipient Color control cluster.
 * @param commandData Struct containing the parameters of the command.
 * @return Status::Success when successful,
 *         Status::InvalidCommand when a rate of 0 for a non-stop move or an unknown HueMoveMode is provided
 *         Status::UnsupportedEndpoint when the provided endpoint doesn't correspond with a color temp transition state.
 *         Status::ConstraintError when a command parameter is outside its defined value range.
 */
Status ColorControlServer::moveColorTempCommand(EndpointId endpoint,
                                                const Commands::MoveColorTemperature::DecodableType & commandData)
{
    // Command Parameters constraint checks:
    VerifyOrReturnValue(commandData.colorTemperatureMinimumMireds <= kMaxColorTemperatureMireds, Status::ConstraintError);
    VerifyOrReturnValue(commandData.colorTemperatureMaximumMireds <= kMaxColorTemperatureMireds, Status::ConstraintError);

    // check moveMode and rate before any operation is done on the transition states
    // rate value is ignored if the MoveMode is stop
    VerifyOrReturnValue(commandData.moveMode != HueMoveMode::kUnknownEnumValue, Status::InvalidCommand);
    VerifyOrReturnValue((commandData.rate != 0 || commandData.moveMode == HueMoveMode::kStop), Status::InvalidCommand);

    Color16uTransitionState * colorTempTransitionState = getTempTransitionState(endpoint);
    VerifyOrReturnValue(colorTempTransitionState != nullptr, Status::UnsupportedEndpoint);

    VerifyOrReturnValue(shouldExecuteIfOff(endpoint, commandData.optionsMask, commandData.optionsOverride), Status::Success);

    uint16_t tempPhysicalMin = MIN_TEMPERATURE_VALUE;
    uint16_t tempPhysicalMax = MAX_TEMPERATURE_VALUE;
    Attributes::ColorTempPhysicalMinMireds::Get(endpoint, &tempPhysicalMin);
    Attributes::ColorTempPhysicalMaxMireds::Get(endpoint, &tempPhysicalMax);

    // Avoid potential divide-by-zero in future Kelvin conversions.
    tempPhysicalMin = std::max(static_cast<uint16_t>(1u), tempPhysicalMin);

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);
    // For HueMoveMode::kStop we are done here.
    VerifyOrReturnValue(commandData.moveMode != HueMoveMode::kStop, Status::Success);

    // Per spec, colorTemperatureMinimumMireds field is limited to ColorTempPhysicalMinMireds and
    // when colorTemperatureMinimumMireds field is 0, ColorTempPhysicalMinMireds shall be used (always > 0)
    uint16_t colorTemperatureMinimum = commandData.colorTemperatureMinimumMireds;
    if (colorTemperatureMinimum < tempPhysicalMin)
    {
        colorTemperatureMinimum = tempPhysicalMin;
    }

    // Per spec, colorTemperatureMaximumMireds field is limited to ColorTempPhysicalMaxMireds and
    // when colorTemperatureMaximumMireds field is 0, ColorTempPhysicalMaxMireds shall be used
    uint16_t colorTemperatureMaximum = commandData.colorTemperatureMaximumMireds;
    if ((colorTemperatureMaximum == 0) || (colorTemperatureMaximum > tempPhysicalMax))
    {
        colorTemperatureMaximum = tempPhysicalMax;
    }

    // Handle color mode transition, if necessary.
    handleModeSwitch(endpoint, EnhancedColorMode::kColorTemperatureMireds);

    // now, kick off the state machine.
    colorTempTransitionState->initialValue = 0;
    Attributes::ColorTemperatureMireds::Get(endpoint, &colorTempTransitionState->initialValue);
    colorTempTransitionState->currentValue = colorTempTransitionState->initialValue;

    if (commandData.moveMode == HueMoveMode::kUp)
    {
        if (tempPhysicalMax > colorTemperatureMaximum)
        {
            colorTempTransitionState->finalValue = colorTemperatureMaximum;
        }
        else
        {
            colorTempTransitionState->finalValue = tempPhysicalMax;
        }
    }
    else
    {
        if (tempPhysicalMin < colorTemperatureMinimum)
        {
            colorTempTransitionState->finalValue = colorTemperatureMinimum;
        }
        else
        {
            colorTempTransitionState->finalValue = tempPhysicalMin;
        }
    }

    uint16_t transitionTime                  = computeTransitionTimeFromStateAndRate(colorTempTransitionState, commandData.rate);
    colorTempTransitionState->stepsRemaining = transitionTime;
    colorTempTransitionState->stepsTotal     = transitionTime;
    colorTempTransitionState->timeRemaining  = transitionTime;
    colorTempTransitionState->transitionTime = transitionTime;
    colorTempTransitionState->endpoint       = endpoint;
    colorTempTransitionState->lowLimit       = colorTemperatureMinimum;
    colorTempTransitionState->highLimit      = colorTemperatureMaximum;

    SetQuietReportRemainingTime(endpoint, transitionTime, true /* isNewTransition */);

    // kick off the state machine:
    scheduleTimerCallbackMs(configureTempEventControl(endpoint), TRANSITION_UPDATE_TIME_MS.count());
    return Status::Success;
}

/**
 * @brief Executes move to color temp command.
 * @param endpoint EndpointId of the recipient Color control cluster.
 * @param commandData Struct containing the parameters of the command.
 * @return Status::Success when successful,
 *         Status::UnsupportedEndpoint when the provided endpoint doesn't correspond with a color XY transition state (verified in
 * moveToColorTemp function),
 *         Status::ConstraintError when a command parameter is outside its defined value range.
 */
Status ColorControlServer::moveToColorTempCommand(EndpointId endpoint,
                                                  const Commands::MoveToColorTemperature::DecodableType & commandData)
{
    // Command Parameters constraint checks:
    VerifyOrReturnValue(commandData.colorTemperatureMireds <= kMaxColorTemperatureMireds, Status::ConstraintError);
    VerifyOrReturnValue(commandData.transitionTime <= kMaxTransitionTime, Status::ConstraintError);

    VerifyOrReturnValue(shouldExecuteIfOff(endpoint, commandData.optionsMask, commandData.optionsOverride), Status::Success);

    Status status = moveToColorTemp(endpoint, commandData.colorTemperatureMireds, commandData.transitionTime);
#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
    ScenesManagement::ScenesServer::Instance().MakeSceneInvalidForAllFabrics(endpoint);
#endif // MATTER_DM_PLUGIN_SCENES_MANAGEMENT
    return status;
}

/**
 * @brief Executes step color temp command.
 * @param endpoint EndpointId of the recipient Color control cluster.
 * @param commandData Struct containing the parameters of the command
 * @return Status::Success when successful,
 *         Status::InvalidCommand when stepSize is 0 or an unknown stepMode is provided
 *         Status::UnsupportedEndpoint when the provided endpoint doesn't correspond with a color temp transition state,
 *         Status::ConstraintError when a command parameter is outside its defined value range.
 */
Status ColorControlServer::stepColorTempCommand(EndpointId endpoint,
                                                const Commands::StepColorTemperature::DecodableType & commandData)
{
    // Command Parameters constraint checks:
    VerifyOrReturnValue(commandData.transitionTime <= kMaxTransitionTime, Status::ConstraintError);
    VerifyOrReturnValue(commandData.colorTemperatureMinimumMireds <= kMaxColorTemperatureMireds, Status::ConstraintError);
    VerifyOrReturnValue(commandData.colorTemperatureMaximumMireds <= kMaxColorTemperatureMireds, Status::ConstraintError);

    // Confirm validity of the step mode and step size received
    VerifyOrReturnValue(commandData.stepMode != HueStepMode::kUnknownEnumValue, Status::InvalidCommand);
    VerifyOrReturnValue(commandData.stepSize != 0, Status::InvalidCommand);

    Color16uTransitionState * colorTempTransitionState = getTempTransitionState(endpoint);
    VerifyOrReturnValue(colorTempTransitionState != nullptr, Status::UnsupportedEndpoint);

    VerifyOrReturnValue(shouldExecuteIfOff(endpoint, commandData.optionsMask, commandData.optionsOverride), Status::Success);

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    uint16_t tempPhysicalMin = MIN_TEMPERATURE_VALUE;
    uint16_t tempPhysicalMax = MAX_TEMPERATURE_VALUE;
    Attributes::ColorTempPhysicalMinMireds::Get(endpoint, &tempPhysicalMin);
    Attributes::ColorTempPhysicalMaxMireds::Get(endpoint, &tempPhysicalMax);

    // Avoid potential divide-by-zero in future Kelvin conversions.
    tempPhysicalMin = std::max(static_cast<uint16_t>(1u), tempPhysicalMin);

    // Per spec, colorTemperatureMinimumMireds field is limited to ColorTempPhysicalMinMireds and
    // when colorTemperatureMinimumMireds field is 0, ColorTempPhysicalMinMireds shall be used (always > 0)
    uint16_t colorTemperatureMinimum = commandData.colorTemperatureMinimumMireds;
    if (colorTemperatureMinimum < tempPhysicalMin)
    {
        colorTemperatureMinimum = tempPhysicalMin;
    }

    // Per spec, colorTemperatureMaximumMireds field is limited to ColorTempPhysicalMaxMireds and
    // when colorTemperatureMaximumMireds field is 0, ColorTempPhysicalMaxMireds shall be used
    uint16_t colorTemperatureMaximum = commandData.colorTemperatureMaximumMireds;
    if ((colorTemperatureMaximum == 0) || (colorTemperatureMaximum > tempPhysicalMax))
    {
        colorTemperatureMaximum = tempPhysicalMax;
    }

    // Handle color mode transition, if necessary.
    handleModeSwitch(endpoint, EnhancedColorMode::kColorTemperatureMireds);

    // now, kick off the state machine.
    colorTempTransitionState->initialValue = 0;
    Attributes::ColorTemperatureMireds::Get(endpoint, &colorTempTransitionState->initialValue);
    colorTempTransitionState->initialValue = std::max(static_cast<uint16_t>(1u), colorTempTransitionState->initialValue);

    colorTempTransitionState->currentValue = colorTempTransitionState->initialValue;

    if (commandData.stepMode == HueStepMode::kUp)
    {
        uint32_t finalValue32u =
            static_cast<uint32_t>(colorTempTransitionState->initialValue) + static_cast<uint32_t>(commandData.stepSize);
        if (finalValue32u > UINT16_MAX)
        {
            colorTempTransitionState->finalValue = UINT16_MAX;
        }
        else
        {
            colorTempTransitionState->finalValue = static_cast<uint16_t>(finalValue32u);
        }
    }
    else if (commandData.stepMode == HueStepMode::kDown)
    {
        uint32_t finalValue32u =
            static_cast<uint32_t>(colorTempTransitionState->initialValue) - static_cast<uint32_t>(commandData.stepSize);
        if (finalValue32u > UINT16_MAX)
        {
            colorTempTransitionState->finalValue = 0;
        }
        else
        {
            colorTempTransitionState->finalValue = static_cast<uint16_t>(finalValue32u);
        }
    }
    colorTempTransitionState->stepsRemaining = std::max<uint16_t>(commandData.transitionTime, 1);
    colorTempTransitionState->stepsTotal     = colorTempTransitionState->stepsRemaining;
    colorTempTransitionState->timeRemaining  = commandData.transitionTime;
    colorTempTransitionState->transitionTime = commandData.transitionTime;
    colorTempTransitionState->endpoint       = endpoint;
    colorTempTransitionState->lowLimit       = colorTemperatureMinimum;
    colorTempTransitionState->highLimit      = colorTemperatureMaximum;

    SetQuietReportRemainingTime(endpoint, commandData.transitionTime, true /* isNewTransition */);

    // kick off the state machine:
    scheduleTimerCallbackMs(configureTempEventControl(endpoint),
                            commandData.transitionTime ? TRANSITION_UPDATE_TIME_MS.count() : 0);
    return Status::Success;
}

void ColorControlServer::levelControlColorTempChangeCommand(EndpointId endpoint)
{
    // ZCL 5.2.2.1.1 Coupling color temperature to Level Control
    //
    // If the Level Control for Lighting cluster identifier 0x0008 is supported
    // on the same endpoint as the Color Control cluster and color temperature is
    // supported, it is possible to couple changes in the current level to the
    // color temperature.
    //
    // The CoupleColorTempToLevel bit of the Options attribute of the Level
    // Control cluster indicates whether the color temperature is to be linked
    // with the CurrentLevel attribute in the Level Control cluster.
    //
    // If the CoupleColorTempToLevel bit of the Options attribute of the Level
    // Control cluster is equal to 1 and the ColorMode or EnhancedColorMode
    // attribute is set to 0x02 (color temperature) then a change in the
    // CurrentLevel attribute SHALL affect the ColorTemperatureMireds attribute.
    // This relationship is manufacturer specific, with the qualification that
    // the maximum value of the CurrentLevel attribute SHALL correspond to a
    // ColorTemperatureMired attribute value equal to the
    // CoupleColorTempToLevelMinMireds attribute. This relationship is one-way so
    // a change to the ColorTemperatureMireds attribute SHALL NOT have any effect
    // on the CurrentLevel attribute.
    //
    // In order to simulate the behavior of an incandescent bulb, a low value of
    // the CurrentLevel attribute SHALL be associated with a high value of the
    // ColorTemperatureMireds attribute (i.e., a low value of color temperature
    // in kelvins).
    //
    // If the CoupleColorTempToLevel bit of the Options attribute of the Level
    // Control cluster is equal to 0, there SHALL be no link between color
    // temperature and current level.

    if (!emberAfContainsServer(endpoint, ColorControl::Id))
    {
        return;
    }

    auto colorMode = ColorModeEnum::kCurrentHueAndCurrentSaturation;
    Attributes::ColorMode::Get(endpoint, &colorMode);

    if (static_cast<EnhancedColorModeEnum>(colorMode) == EnhancedColorMode::kColorTemperatureMireds)
    {
        app::DataModel::Nullable<uint8_t> currentLevel;
        Status status = LevelControl::Attributes::CurrentLevel::Get(endpoint, currentLevel);

        if (status != Status::Success || currentLevel.IsNull())
        {
            currentLevel.SetNonNull((uint8_t) 0x7F);
        }

        uint16_t tempCoupleMin = getTemperatureCoupleToLevelMin(endpoint);
        uint16_t tempPhysMax   = MAX_TEMPERATURE_VALUE;
        Attributes::ColorTempPhysicalMaxMireds::Get(endpoint, &tempPhysMax);

        // Scale color temp setting between the coupling min and the physical max.
        // Note that mireds varies inversely with level: low level -> high mireds.
        // Peg min/MAX level to MAX/min mireds, otherwise interpolate.
        uint16_t newColorTemp;
        if (currentLevel.Value() <= MIN_CURRENT_LEVEL)
        {
            newColorTemp = tempPhysMax;
        }
        else if (currentLevel.Value() >= MAX_CURRENT_LEVEL)
        {
            newColorTemp = tempCoupleMin;
        }
        else
        {
            uint32_t u32TempPhysMax = static_cast<uint32_t>(tempPhysMax); // use a u32 to prevent overflows in next steps.
            uint32_t tempDelta =
                ((u32TempPhysMax - tempCoupleMin) * currentLevel.Value()) / (MAX_CURRENT_LEVEL - MIN_CURRENT_LEVEL + 1);

            newColorTemp = static_cast<uint16_t>(tempPhysMax - tempDelta);
        }

        // Apply new color temp.
        moveToColorTemp(endpoint, newColorTemp, 0);
    }
}

#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP

/*
 * @brief
 * Utility function used to update a color control attribute which has the quiet reporting quality.
 * matching the following report conditions:
 * - At most once per second, or
 * - At the end of the movement/transition, or
 * - When it changes from null to any other value and vice versa. (Implicit to the QuieterReportingAttribute class)
 *
 * The QuietReportAttribute class is updated with the new value and when the report conditions are met,
 * this function will return MarkAttributeDirty::kYes.
 * It is expected that the user will use this return value to trigger a reporting mechanism for the attribute with the new value
 * (Which was updated in the quietReporter)
 *
 * @param quietReporter: The QuieterReportingAttribute<TYPE> object for the attribute to update.
 * @param newValue: Value to update the attribute with
 * @param isEndOfTransition: Boolean that indicates whether the update is occurring at the end of a color transition
 * @return MarkAttributeDirty::kYes when the attribute must be marked dirty and be reported. MarkAttributeDirty::kNo when
 * no report is needed.
 */
template <typename Q, typename V>
MarkAttributeDirty ColorControlServer::SetQuietReportAttribute(QuieterReportingAttribute<Q> & quietReporter, V newValue,
                                                               bool isEndOfTransition, uint16_t transitionTime)
{
    AttributeDirtyState dirtyState;
    auto now = System::SystemClock().GetMonotonicTimestamp();

    if (isEndOfTransition)
    {
        // At the end of the movement/transition we must report if the value changed
        auto predicate = [](const typename QuieterReportingAttribute<Q>::SufficientChangePredicateCandidate &) -> bool {
            return true;
        };
        dirtyState = quietReporter.SetValue(newValue, now, predicate);
    }
    else
    {
        // During transitions, reports should be at most once per second

        // For "infinite" transition, default reports interval to 10s (100 1/10ths of a second )
        if (transitionTime == MAX_INT16U_VALUE)
        {
            transitionTime = 100;
        }

        // Opt for the longest interval between reports, 1s or (transitionTime / 4).
        // Since transitionTime is in 1/10th of a second, convert it to ms (x 100), thus * 100/4 -> * 25
        System::Clock::Milliseconds64 reportInterval = System::Clock::Milliseconds64(std::max(1000, transitionTime * 25));
        auto predicate                               = quietReporter.GetPredicateForSufficientTimeSinceLastDirty(reportInterval);
        dirtyState                                   = quietReporter.SetValue(newValue, now, predicate);
    }

    return (dirtyState == AttributeDirtyState::kMustReport) ? MarkAttributeDirty::kYes : MarkAttributeDirty::kNo;
}

/*
 * @brief
 * Function used to set the remaining time based on quiet reporting conditions.
 * It will update the attribute storage and report the attribute if it is determined dirty.
 * The conditions on which the attribute must be reported are:
 * - When it changes from 0 to any value higher than 10, or
 * - When it changes, with a delta larger than 10, caused by the invoke of a command, or
 * - When it changes to 0.
 *
 * @param endpoint: Endpoint of the RemainingTime attribute to set
 * @param newRemainingTime: Value to update the RemainingTime attribute with
 * @return Success in setting the attribute value or the IM error code for the failure.
 */
Status ColorControlServer::SetQuietReportRemainingTime(EndpointId endpoint, uint16_t newRemainingTime, bool isNewTransition)
{
    uint16_t epIndex           = getEndpointIndex(endpoint);
    uint16_t lastRemainingTime = quietRemainingTime[epIndex].value().ValueOr(0);
    auto markDirty             = MarkAttributeDirty::kNo;
    auto now                   = System::SystemClock().GetMonotonicTimestamp();

    auto predicate =
        [isNewTransition, lastRemainingTime](
            const typename QuieterReportingAttribute<uint16_t>::SufficientChangePredicateCandidate & candidate) -> bool {
        constexpr uint16_t reportDelta = 10;
        bool isDirty                   = false;
        if (candidate.newValue.Value() == 0 || (candidate.lastDirtyValue.Value() == 0 && candidate.newValue.Value() > reportDelta))
        {
            isDirty = true;
        }
        else if (isNewTransition &&
                 (candidate.newValue.Value() > static_cast<uint32_t>(lastRemainingTime + reportDelta) ||
                  static_cast<uint32_t>(candidate.newValue.Value() + reportDelta) < lastRemainingTime ||
                  candidate.newValue.Value() > static_cast<uint32_t>(candidate.lastDirtyValue.Value() + reportDelta)))
        {
            isDirty = true;
        }
        return isDirty;
    };

    if (quietRemainingTime[epIndex].SetValue(newRemainingTime, now, predicate) == AttributeDirtyState::kMustReport)
    {
        markDirty = MarkAttributeDirty::kYes;
    }

    return Attributes::RemainingTime::Set(endpoint, quietRemainingTime[epIndex].value().Value(), markDirty);
}

/**********************************************************
 * Callbacks Implementation
 *********************************************************/

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV

bool emberAfColorControlClusterMoveHueCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                               const Commands::MoveHue::DecodableType & commandData)
{
    Status status = ColorControlServer::Instance().moveHueCommand(commandPath.mEndpointId, commandData.moveMode, commandData.rate,
                                                                  commandData.optionsMask, commandData.optionsOverride, false);
    commandObj->AddStatus(commandPath, status);
    return true;
}

bool emberAfColorControlClusterMoveSaturationCallback(app::CommandHandler * commandObj,
                                                      const app::ConcreteCommandPath & commandPath,
                                                      const Commands::MoveSaturation::DecodableType & commandData)
{
    Status status = ColorControlServer::Instance().moveSaturationCommand(commandPath.mEndpointId, commandData);
    commandObj->AddStatus(commandPath, status);
    return true;
}

bool emberAfColorControlClusterMoveToHueCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                 const Commands::MoveToHue::DecodableType & commandData)
{
    Status status = ColorControlServer::Instance().moveToHueCommand(commandPath.mEndpointId, commandData.hue, commandData.direction,
                                                                    commandData.transitionTime, commandData.optionsMask,
                                                                    commandData.optionsOverride, false);
    commandObj->AddStatus(commandPath, status);
    return true;
}

bool emberAfColorControlClusterMoveToSaturationCallback(app::CommandHandler * commandObj,
                                                        const app::ConcreteCommandPath & commandPath,
                                                        const Commands::MoveToSaturation::DecodableType & commandData)
{
    Status status = ColorControlServer::Instance().moveToSaturationCommand(commandPath.mEndpointId, commandData);
    commandObj->AddStatus(commandPath, status);
    return true;
}

bool emberAfColorControlClusterMoveToHueAndSaturationCallback(app::CommandHandler * commandObj,
                                                              const app::ConcreteCommandPath & commandPath,
                                                              const Commands::MoveToHueAndSaturation::DecodableType & commandData)
{
    Status status = ColorControlServer::Instance().moveToHueAndSaturationCommand(
        commandPath.mEndpointId, commandData.hue, commandData.saturation, commandData.transitionTime, commandData.optionsMask,
        commandData.optionsOverride, false);
    commandObj->AddStatus(commandPath, status);
    return true;
}

bool emberAfColorControlClusterStepHueCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                               const Commands::StepHue::DecodableType & commandData)
{
    Status status = ColorControlServer::Instance().stepHueCommand(commandPath.mEndpointId, commandData.stepMode,
                                                                  commandData.stepSize, commandData.transitionTime,
                                                                  commandData.optionsMask, commandData.optionsOverride, false);
    commandObj->AddStatus(commandPath, status);
    return true;
}

bool emberAfColorControlClusterStepSaturationCallback(app::CommandHandler * commandObj,
                                                      const app::ConcreteCommandPath & commandPath,
                                                      const Commands::StepSaturation::DecodableType & commandData)
{
    Status status = ColorControlServer::Instance().stepSaturationCommand(commandPath.mEndpointId, commandData);
    commandObj->AddStatus(commandPath, status);
    return true;
}

bool emberAfColorControlClusterEnhancedMoveHueCallback(app::CommandHandler * commandObj,
                                                       const app::ConcreteCommandPath & commandPath,
                                                       const Commands::EnhancedMoveHue::DecodableType & commandData)
{
    Status status = ColorControlServer::Instance().moveHueCommand(commandPath.mEndpointId, commandData.moveMode, commandData.rate,
                                                                  commandData.optionsMask, commandData.optionsOverride, true);
    commandObj->AddStatus(commandPath, status);
    return true;
}

bool emberAfColorControlClusterEnhancedMoveToHueCallback(app::CommandHandler * commandObj,
                                                         const app::ConcreteCommandPath & commandPath,
                                                         const Commands::EnhancedMoveToHue::DecodableType & commandData)
{
    Status status = ColorControlServer::Instance().moveToHueCommand(commandPath.mEndpointId, commandData.enhancedHue,
                                                                    commandData.direction, commandData.transitionTime,
                                                                    commandData.optionsMask, commandData.optionsOverride, true);
    commandObj->AddStatus(commandPath, status);
    return true;
}

bool emberAfColorControlClusterEnhancedMoveToHueAndSaturationCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::EnhancedMoveToHueAndSaturation::DecodableType & commandData)
{
    Status status = ColorControlServer::Instance().moveToHueAndSaturationCommand(
        commandPath.mEndpointId, commandData.enhancedHue, commandData.saturation, commandData.transitionTime,
        commandData.optionsMask, commandData.optionsOverride, true);
    commandObj->AddStatus(commandPath, status);
    return true;
}

bool emberAfColorControlClusterEnhancedStepHueCallback(app::CommandHandler * commandObj,
                                                       const app::ConcreteCommandPath & commandPath,
                                                       const Commands::EnhancedStepHue::DecodableType & commandData)
{
    Status status = ColorControlServer::Instance().stepHueCommand(commandPath.mEndpointId, commandData.stepMode,
                                                                  commandData.stepSize, commandData.transitionTime,
                                                                  commandData.optionsMask, commandData.optionsOverride, true);
    commandObj->AddStatus(commandPath, status);
    return true;
}

bool emberAfColorControlClusterColorLoopSetCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                    const Commands::ColorLoopSet::DecodableType & commandData)
{
    Status status = ColorControlServer::Instance().colorLoopCommand(commandPath.mEndpointId, commandData);
    commandObj->AddStatus(commandPath, status);
    return true;
}

#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_XY

bool emberAfColorControlClusterMoveToColorCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                   const Commands::MoveToColor::DecodableType & commandData)
{
    Status status = ColorControlServer::Instance().moveToColorCommand(commandPath.mEndpointId, commandData);
    commandObj->AddStatus(commandPath, status);
    return true;
}

bool emberAfColorControlClusterMoveColorCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                 const Commands::MoveColor::DecodableType & commandData)
{
    Status status = ColorControlServer::Instance().moveColorCommand(commandPath.mEndpointId, commandData);
    commandObj->AddStatus(commandPath, status);
    return true;
}

bool emberAfColorControlClusterStepColorCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                 const Commands::StepColor::DecodableType & commandData)
{
    Status status = ColorControlServer::Instance().stepColorCommand(commandPath.mEndpointId, commandData);
    commandObj->AddStatus(commandPath, status);
    return true;
}

#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_XY

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP

bool emberAfColorControlClusterMoveToColorTemperatureCallback(app::CommandHandler * commandObj,
                                                              const app::ConcreteCommandPath & commandPath,
                                                              const Commands::MoveToColorTemperature::DecodableType & commandData)
{
    Status status = ColorControlServer::Instance().moveToColorTempCommand(commandPath.mEndpointId, commandData);
    commandObj->AddStatus(commandPath, status);
    return true;
}

bool emberAfColorControlClusterMoveColorTemperatureCallback(app::CommandHandler * commandObj,
                                                            const app::ConcreteCommandPath & commandPath,
                                                            const Commands::MoveColorTemperature::DecodableType & commandData)
{
    Status status = ColorControlServer::Instance().moveColorTempCommand(commandPath.mEndpointId, commandData);
    commandObj->AddStatus(commandPath, status);
    return true;
}

bool emberAfColorControlClusterStepColorTemperatureCallback(app::CommandHandler * commandObj,
                                                            const app::ConcreteCommandPath & commandPath,
                                                            const Commands::StepColorTemperature::DecodableType & commandData)
{
    Status status = ColorControlServer::Instance().stepColorTempCommand(commandPath.mEndpointId, commandData);
    commandObj->AddStatus(commandPath, status);
    return true;
}

void emberAfPluginLevelControlCoupledColorTempChangeCallback(EndpointId endpoint)
{
    ColorControlServer::Instance().levelControlColorTempChangeCommand(endpoint);
}

#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP

bool emberAfColorControlClusterStopMoveStepCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                    const Commands::StopMoveStep::DecodableType & commandData)
{
    Status status = ColorControlServer::Instance().stopMoveStepCommand(commandPath.mEndpointId, commandData);
    commandObj->AddStatus(commandPath, status);
    return true;
}

void emberAfColorControlClusterServerInitCallback(EndpointId endpoint)
{
#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP
    ColorControlServer::Instance().startUpColorTempCommand(endpoint);
#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP
#if defined(MATTER_DM_PLUGIN_SCENES_MANAGEMENT) && CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS
    // Registers Scene handlers for the color control cluster on the server
    app::Clusters::ScenesManagement::ScenesServer::Instance().RegisterSceneHandler(
        endpoint, ColorControlServer::Instance().GetSceneHandler());
#endif // defined(MATTER_DM_PLUGIN_SCENES_MANAGEMENT) && CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS
}

void MatterColorControlClusterServerShutdownCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "Shuting down color control server cluster on endpoint %d", endpoint);
    ColorControlServer::Instance().cancelEndpointTimerCallback(endpoint);
}

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP
/**
 * @brief Callback for temperature update when timer is finished
 *
 * @param endpoint
 */
void emberAfPluginColorControlServerTempTransitionEventHandler(EndpointId endpoint)
{
    ColorControlServer::Instance().updateTempCommand(endpoint);
}
#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_XY
/**
 * @brief Callback for color update when timer is finished
 *
 * @param endpoint
 */
void emberAfPluginColorControlServerXyTransitionEventHandler(EndpointId endpoint)
{
    ColorControlServer::Instance().updateXYCommand(endpoint);
}
#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_XY

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV
/**
 * @brief Callback for color hue and saturation update when timer is finished
 *
 * @param endpoint
 */
void emberAfPluginColorControlServerHueSatTransitionEventHandler(EndpointId endpoint)
{
    ColorControlServer::Instance().updateHueSatCommand(endpoint);
}
#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV

void MatterColorControlPluginServerInitCallback() {}
