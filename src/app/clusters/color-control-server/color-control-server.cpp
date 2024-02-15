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

#include "color-control-server.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
#include <tracing/macros.h>

#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
#include <app/clusters/scenes-server/scenes-server.h>
#endif

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ColorControl;
using chip::Protocols::InteractionModel::Status;

// These constants are NOT currently spec compliant
// These should be changed once we have real specification enumeration
// names.
namespace chip {
namespace app {
namespace Clusters {
namespace ColorControl {

namespace EnhancedColorMode {
constexpr uint8_t kCurrentHueAndCurrentSaturation = ColorControlServer::EnhancedColorMode::kCurrentHueAndCurrentSaturation;
constexpr uint8_t kCurrentXAndCurrentY            = ColorControlServer::EnhancedColorMode::kCurrentXAndCurrentY;
constexpr uint8_t kColorTemperature               = ColorControlServer::EnhancedColorMode::kColorTemperature;
constexpr uint8_t kEnhancedCurrentHueAndCurrentSaturation =
    ColorControlServer::EnhancedColorMode::kEnhancedCurrentHueAndCurrentSaturation;
} // namespace EnhancedColorMode

namespace Options {
constexpr uint8_t kExecuteIfOff = 1;
} // namespace Options

} // namespace ColorControl
} // namespace Clusters
} // namespace app
} // namespace chip

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
        using AttributeValuePair = ScenesManagement::Structs::AttributeValuePair::Type;

        AttributeValuePair pairs[kColorControlScenableAttributesCount];

        size_t attributeCount = 0;

        if (ColorControlServer::Instance().HasFeature(endpoint, ColorControlServer::Feature::kXy))
        {
            uint16_t xValue;
            if (Status::Success != Attributes::CurrentX::Get(endpoint, &xValue))
            {
                xValue = 0x616B; // Default X value according to spec
            }
            AddAttributeValuePair(pairs, Attributes::CurrentX::Id, xValue, attributeCount);

            uint16_t yValue;
            if (Status::Success != Attributes::CurrentY::Get(endpoint, &yValue))
            {
                yValue = 0x607D; // Default Y value according to spec
            }
            AddAttributeValuePair(pairs, Attributes::CurrentY::Id, yValue, attributeCount);
        }

        if (ColorControlServer::Instance().HasFeature(endpoint, ColorControlServer::Feature::kEnhancedHue))
        {
            uint16_t hueValue = 0x0000;
            Attributes::EnhancedCurrentHue::Get(endpoint, &hueValue);
            AddAttributeValuePair(pairs, Attributes::EnhancedCurrentHue::Id, hueValue, attributeCount);
        }

        if (ColorControlServer::Instance().HasFeature(endpoint, ColorControlServer::Feature::kHueAndSaturation))
        {
            uint8_t saturationValue;
            if (Status::Success != Attributes::CurrentSaturation::Get(endpoint, &saturationValue))
            {
                saturationValue = 0x00;
            }
            AddAttributeValuePair(pairs, Attributes::CurrentSaturation::Id, saturationValue, attributeCount);
        }

        if (ColorControlServer::Instance().HasFeature(endpoint, ColorControlServer::Feature::kColorLoop))
        {
            uint8_t loopActiveValue;
            if (Status::Success != Attributes::ColorLoopActive::Get(endpoint, &loopActiveValue))
            {
                loopActiveValue = 0x00;
            }
            AddAttributeValuePair(pairs, Attributes::ColorLoopActive::Id, loopActiveValue, attributeCount);

            uint8_t loopDirectionValue;
            if (Status::Success != Attributes::ColorLoopDirection::Get(endpoint, &loopDirectionValue))
            {
                loopDirectionValue = 0x00;
            }
            AddAttributeValuePair(pairs, Attributes::ColorLoopDirection::Id, loopDirectionValue, attributeCount);

            uint16_t loopTimeValue;
            if (Status::Success != Attributes::ColorLoopTime::Get(endpoint, &loopTimeValue))
            {
                loopTimeValue = 0x0019; // Default loop time value according to spec
            }
            AddAttributeValuePair(pairs, Attributes::ColorLoopTime::Id, loopTimeValue, attributeCount);
        }

        if (ColorControlServer::Instance().HasFeature(endpoint, ColorControlServer::Feature::kColorTemperature))
        {
            uint16_t temperatureValue;
            if (Status::Success != Attributes::ColorTemperatureMireds::Get(endpoint, &temperatureValue))
            {
                temperatureValue = 0x00FA; // Default temperature value according to spec
            }
            AddAttributeValuePair(pairs, Attributes::ColorTemperatureMireds::Id, temperatureValue, attributeCount);
        }

        uint8_t modeValue;
        if (Status::Success != Attributes::EnhancedColorMode::Get(endpoint, &modeValue))
        {
            modeValue = ColorControl::EnhancedColorMode::kCurrentXAndCurrentY; // Default mode value according to spec
        }
        AddAttributeValuePair(pairs, Attributes::EnhancedColorMode::Id, modeValue, attributeCount);

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
        app::DataModel::DecodableList<ScenesManagement::Structs::AttributeValuePair::DecodableType> attributeValueList;

        ReturnErrorOnFailure(DecodeAttributeValueList(serializedBytes, attributeValueList));

        size_t attributeCount = 0;
        auto pair_iterator    = attributeValueList.begin();

        // The color control cluster should have a maximum of 9 scenable attributes
        ReturnErrorOnFailure(attributeValueList.ComputeSize(&attributeCount));
        VerifyOrReturnError(attributeCount <= kColorControlScenableAttributesCount, CHIP_ERROR_BUFFER_TOO_SMALL);
        // Retrieve the buffers for different modes
#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV
        ColorControlServer::ColorHueTransitionState * colorHueTransitionState =
            ColorControlServer::Instance().getColorHueTransitionState(endpoint);
        ColorControlServer::Color16uTransitionState * colorSaturationTransitionState =
            ColorControlServer::Instance().getSaturationTransitionState(endpoint);
#endif
#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_XY
        ColorControlServer::Color16uTransitionState * colorXTransitionState =
            ColorControlServer::Instance().getXTransitionState(endpoint);
        ColorControlServer::Color16uTransitionState * colorYTransitionState =
            ColorControlServer::Instance().getYTransitionState(endpoint);
#endif
#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP
        ColorControlServer::Color16uTransitionState * colorTempTransitionState =
            ColorControlServer::Instance().getTempTransitionState(endpoint);
#endif

        // Initialize action attributes to default values in case they are not in the scene
        uint8_t targetColorMode    = 0x00;
        uint8_t loopActiveValue    = 0x00;
        uint8_t loopDirectionValue = 0x00;
        uint16_t loopTimeValue     = 0x0019; // Default loop time value according to spec

        while (pair_iterator.Next())
        {
            auto & decodePair = pair_iterator.GetValue();

            switch (decodePair.attributeID)
            {
            case Attributes::CurrentX::Id:
                if (SupportsColorMode(endpoint, ColorControl::EnhancedColorMode::kCurrentXAndCurrentY))
                {
                    if (decodePair.attributeValue)
                        colorXTransitionState->finalValue =
                            std::min(static_cast<uint16_t>(decodePair.attributeValue), colorXTransitionState->highLimit);
                }
                break;
            case Attributes::CurrentY::Id:
                if (SupportsColorMode(endpoint, ColorControl::EnhancedColorMode::kCurrentXAndCurrentY))
                {
                    colorYTransitionState->finalValue =
                        std::min(static_cast<uint16_t>(decodePair.attributeValue), colorYTransitionState->highLimit);
                }
                break;
            case Attributes::EnhancedCurrentHue::Id:
                if (SupportsColorMode(endpoint, ColorControl::EnhancedColorMode::kEnhancedCurrentHueAndCurrentSaturation))
                {
                    colorHueTransitionState->finalEnhancedHue = static_cast<uint16_t>(decodePair.attributeValue);
                }
                break;
            case Attributes::CurrentSaturation::Id:
                if (SupportsColorMode(endpoint, ColorControl::EnhancedColorMode::kCurrentHueAndCurrentSaturation))
                {
                    colorSaturationTransitionState->finalValue =
                        std::min(static_cast<uint16_t>(decodePair.attributeValue), colorSaturationTransitionState->highLimit);
                }
                break;
            case Attributes::ColorLoopActive::Id:
                loopActiveValue = static_cast<uint8_t>(decodePair.attributeValue);
                break;
            case Attributes::ColorLoopDirection::Id:
                loopDirectionValue = static_cast<uint8_t>(decodePair.attributeValue);
                break;
            case Attributes::ColorLoopTime::Id:
                loopTimeValue = static_cast<uint16_t>(decodePair.attributeValue);
                break;
            case Attributes::ColorTemperatureMireds::Id:
                if (SupportsColorMode(endpoint, ColorControl::EnhancedColorMode::kColorTemperature))
                {
                    colorTempTransitionState->finalValue =
                        std::min(static_cast<uint16_t>(decodePair.attributeValue), colorTempTransitionState->highLimit);
                }
                break;
            case Attributes::EnhancedColorMode::Id:
                if (decodePair.attributeValue <=
                    static_cast<uint8_t>(ColorControl::EnhancedColorMode::kEnhancedCurrentHueAndCurrentSaturation))
                {
                    targetColorMode = static_cast<uint8_t>(decodePair.attributeValue);
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
            case ColorControl::EnhancedColorMode::kCurrentHueAndCurrentSaturation:
#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV
                ColorControlServer::Instance().moveToSaturation(static_cast<uint8_t>(colorSaturationTransitionState->finalValue),
                                                                transitionTime10th, endpoint);
#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV
                break;
            case ColorControl::EnhancedColorMode::kCurrentXAndCurrentY:
#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_XY
                ColorControlServer::Instance().moveToColor(colorXTransitionState->finalValue, colorYTransitionState->finalValue,
                                                           transitionTime10th, endpoint);
#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_XY
                break;
            case ColorControl::EnhancedColorMode::kColorTemperature:
#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP
                ColorControlServer::Instance().moveToColorTemp(
                    endpoint, static_cast<uint16_t>(colorTempTransitionState->finalValue), transitionTime10th);
#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP
                break;
            case ColorControl::EnhancedColorMode::kEnhancedCurrentHueAndCurrentSaturation:
#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV
                ColorControlServer::Instance().moveToHueAndSaturation(
                    colorHueTransitionState->finalEnhancedHue, static_cast<uint8_t>(colorSaturationTransitionState->finalValue),
                    transitionTime10th, true, endpoint);
#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV
                break;
            default:
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
        }
        return CHIP_NO_ERROR;
    }

private:
    bool SupportsColorMode(EndpointId endpoint, uint8_t mode)
    {
        switch (mode)
        {
        case ColorControl::EnhancedColorMode::kCurrentHueAndCurrentSaturation:
            return ColorControlServer::Instance().HasFeature(endpoint, ColorControlServer::Feature::kHueAndSaturation);
            break;
        case ColorControl::EnhancedColorMode::kCurrentXAndCurrentY:
            return ColorControlServer::Instance().HasFeature(endpoint, ColorControlServer::Feature::kXy);
            break;
        case ColorControl::EnhancedColorMode::kColorTemperature:
            return ColorControlServer::Instance().HasFeature(endpoint, ColorControlServer::Feature::kColorTemperature);
            break;
        case ColorControl::EnhancedColorMode::kEnhancedCurrentHueAndCurrentSaturation:
            return ColorControlServer::Instance().HasFeature(endpoint, ColorControlServer::Feature::kEnhancedHue);
            break;
        default:
            return false;
        }
    }

    void AddAttributeValuePair(ScenesManagement::Structs::AttributeValuePair::Type * pairs, AttributeId id, uint32_t value,
                               size_t & attributeCount)
    {
        pairs[attributeCount].attributeID    = id;
        pairs[attributeCount].attributeValue = value;
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

chip::scenes::SceneHandler * ColorControlServer::GetSceneHandler()
{

#if defined(MATTER_DM_PLUGIN_SCENES_MANAGEMENT) && CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS
    return &sColorControlSceneHandler;
#else
    return nullptr;
#endif // defined(MATTER_DM_PLUGIN_SCENES_MANAGEMENT) && CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS
}

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

bool ColorControlServer::stopMoveStepCommand(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                             uint8_t optionsMask, uint8_t optionsOverride)
{
    EndpointId endpoint = commandPath.mEndpointId;
    Status status       = Status::Success;

    if (shouldExecuteIfOff(endpoint, optionsMask, optionsOverride))
    {
        status = stopAllColorTransitions(endpoint);

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV
        // Because Hue and Saturation have separate transitions and can be kicked separately,
        // a new command specific to Hue could resume an old unfinished Saturation transition. Or vice versa.
        // Init both transition states on stop command to prevent that.
        if (status == Status::Success)
        {
            ColorHueTransitionState * hueState        = getColorHueTransitionState(endpoint);
            Color16uTransitionState * saturationState = getSaturationTransitionState(endpoint);
            initHueTransitionState(endpoint, hueState, false /*isEnhancedHue don't care*/);
            initSaturationTransitionState(endpoint, saturationState);
        }
#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV
    }

    commandObj->AddStatus(commandPath, status);
    return true;
}

bool ColorControlServer::shouldExecuteIfOff(EndpointId endpoint, uint8_t optionMask, uint8_t optionOverride)
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

    uint8_t options = 0x00;
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
    if (optionMask == 0xFF && optionOverride == 0xFF)
    {
        // 0xFF are the default values passed to the command handler when
        // the payload is not present - in that case there is use of option
        // attribute to decide execution of the command
        return READBITS(options, ColorControl::Options::kExecuteIfOff);
    }
    // ---------- The above is to distinguish if the payload is present or not

    if (READBITS(optionMask, ColorControl::Options::kExecuteIfOff))
    {
        // Mask is present and set in the command payload, this indicates
        // use the override as temporary option
        return READBITS(optionOverride, ColorControl::Options::kExecuteIfOff);
    }
    // if we are here - use the option attribute bits
    return (READBITS(options, ColorControl::Options::kExecuteIfOff));
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
void ColorControlServer::handleModeSwitch(EndpointId endpoint, uint8_t newColorMode)
{
    uint8_t oldColorMode = 0;
    Attributes::ColorMode::Get(endpoint, &oldColorMode);

    uint8_t colorModeTransition;

    if (oldColorMode == newColorMode)
    {
        return;
    }

    Attributes::EnhancedColorMode::Set(endpoint, newColorMode);
    if (newColorMode == ColorControl::EnhancedColorMode::kEnhancedCurrentHueAndCurrentSaturation)
    {
        // Transpose COLOR_MODE_EHSV to ColorControl::EnhancedColorMode::kCurrentHueAndCurrentSaturation after setting
        // EnhancedColorMode
        newColorMode = ColorControl::EnhancedColorMode::kCurrentHueAndCurrentSaturation;
    }
    Attributes::ColorMode::Set(endpoint, newColorMode);

    colorModeTransition = static_cast<uint8_t>((newColorMode << 4) + oldColorMode);

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
    uint16_t index =
        emberAfGetClusterServerEndpointIndex(endpoint, ColorControl::Id, MATTER_DM_COLOR_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    EmberEventControl * event = nullptr;

    if (index < ArraySize(eventControls))
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
 * @brief Returns ColorHueTransititionState associated to an endpoint
 *
 * @param[in] endpoint
 * @return ColorControlServer::ColorHueTransitionState*
 */
ColorControlServer::ColorHueTransitionState * ColorControlServer::getColorHueTransitionState(EndpointId endpoint)
{
    uint16_t index =
        emberAfGetClusterServerEndpointIndex(endpoint, ColorControl::Id, MATTER_DM_COLOR_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    ColorHueTransitionState * state = nullptr;

    if (index < ArraySize(colorHueTransitionStates))
    {
        state = &colorHueTransitionStates[index];
    }
    return state;
}

/**
 * @brief Returns Color16uTransitionState for saturation associated to an endpoint
 *
 * @param[in] endpoint
 * @return ColorControlServer::Color16uTransitionState*
 */
ColorControlServer::Color16uTransitionState * ColorControlServer::getSaturationTransitionState(EndpointId endpoint)
{
    uint16_t index =
        emberAfGetClusterServerEndpointIndex(endpoint, ColorControl::Id, MATTER_DM_COLOR_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    Color16uTransitionState * state = nullptr;

    if (index < ArraySize(colorSatTransitionStates))
    {
        state = &colorSatTransitionStates[index];
    }
    return state;
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

    if (direction == to_underlying(ColorLoopDirection::kIncrementHue))
    {
        colorHueTransitionState->finalEnhancedHue = static_cast<uint16_t>(startHue - 1);
    }
    else
    {
        colorHueTransitionState->finalEnhancedHue = static_cast<uint16_t>(startHue + 1);
    }

    colorHueTransitionState->up     = (direction == to_underlying(ColorLoopDirection::kIncrementHue));
    colorHueTransitionState->repeat = true;

    colorHueTransitionState->stepsRemaining = static_cast<uint16_t>(time * TRANSITION_STEPS_PER_1S);
    colorHueTransitionState->stepsTotal     = static_cast<uint16_t>(time * TRANSITION_STEPS_PER_1S);
    colorHueTransitionState->timeRemaining  = MAX_INT16U_VALUE;
    colorHueTransitionState->endpoint       = endpoint;

    Attributes::RemainingTime::Set(endpoint, MAX_INT16U_VALUE);

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
    ColorHueTransitionState * hueTransitionState        = getColorHueTransitionState(endpoint);
    Color16uTransitionState * saturationTransitionState = getSaturationTransitionState(endpoint);

    // When the hue transition is loop, RemainingTime stays at MAX_INT16
    if (hueTransitionState->repeat == false)
    {
        Attributes::RemainingTime::Set(endpoint, max(hueTransitionState->timeRemaining, saturationTransitionState->timeRemaining));
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

        // Check if we are in a color loop. If not, we         are in a moveHue
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
 * @brief executes move to saturation command
 *
 * @param saturation target saturation
 * @param transitionTime transition time in 10th of seconds
 * @param endpoint target endpoint where to execute move
 * @return Status::Success if successful,Status::UnsupportedEndpoint if the saturation transition state doesn't exist,
 * Status::ConstraintError if the saturation is above maximum
 */
Status ColorControlServer::moveToSaturation(uint8_t saturation, uint16_t transitionTime, EndpointId endpoint)
{
    Color16uTransitionState * colorSaturationTransitionState = getSaturationTransitionState(endpoint);
    VerifyOrReturnError(nullptr != colorSaturationTransitionState, Status::UnsupportedEndpoint);

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    // Handle color mode transition, if necessary.
    handleModeSwitch(endpoint, ColorControl::EnhancedColorMode::kCurrentHueAndCurrentSaturation);

    // now, kick off the state machine.
    initSaturationTransitionState(endpoint, colorSaturationTransitionState);
    colorSaturationTransitionState->finalValue     = saturation;
    colorSaturationTransitionState->stepsRemaining = max<uint16_t>(transitionTime, 1);
    colorSaturationTransitionState->stepsTotal     = colorSaturationTransitionState->stepsRemaining;
    colorSaturationTransitionState->timeRemaining  = transitionTime;
    colorSaturationTransitionState->endpoint       = endpoint;
    colorSaturationTransitionState->lowLimit       = MIN_SATURATION_VALUE;
    colorSaturationTransitionState->highLimit      = MAX_SATURATION_VALUE;

    SetHSVRemainingTime(endpoint);

    // kick off the state machine:
    scheduleTimerCallbackMs(configureHSVEventControl(endpoint), transitionTime ? TRANSITION_UPDATE_TIME_MS.count() : 0);

    return Status::Success;
}

/**
 * @brief executes move to hue and saturatioan command
 *
 * @param[in] hue target hue
 * @param[in] saturation target saturation
 * @param[in] transitionTime transition time in 10th of seconds
 * @param[in] isEnhanced If True, function was called by EnhancedMoveHue command and rate is a uint16 value. If False function
 * was called by MoveHue command and rate is a uint8 value
 * @param[in] endpoint
 * @return Status::Success if successful,Status::UnsupportedEndpoint if the saturation transition state doesn't exist,
 * Status::ConstraintError if the saturation is above maximum
 */
Status ColorControlServer::moveToHueAndSaturation(uint16_t hue, uint8_t saturation, uint16_t transitionTime, bool isEnhanced,
                                                  EndpointId endpoint)
{
    uint16_t currentHue = 0;
    uint16_t halfWay    = isEnhanced ? HALF_MAX_UINT16T : HALF_MAX_UINT8T;
    bool moveUp;

    Color16uTransitionState * colorSaturationTransitionState = getSaturationTransitionState(endpoint);
    ColorHueTransitionState * colorHueTransitionState        = getColorHueTransitionState(endpoint);

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
    colorHueTransitionState->stepsRemaining = max<uint16_t>(transitionTime, 1);
    colorHueTransitionState->stepsTotal     = colorHueTransitionState->stepsRemaining;
    colorHueTransitionState->timeRemaining  = transitionTime;
    colorHueTransitionState->endpoint       = endpoint;
    colorHueTransitionState->repeat         = false;

    initSaturationTransitionState(endpoint, colorSaturationTransitionState);
    colorSaturationTransitionState->finalValue     = saturation;
    colorSaturationTransitionState->stepsRemaining = colorHueTransitionState->stepsRemaining;
    colorSaturationTransitionState->stepsTotal     = colorHueTransitionState->stepsRemaining;
    colorSaturationTransitionState->timeRemaining  = transitionTime;
    colorSaturationTransitionState->endpoint       = endpoint;
    colorSaturationTransitionState->lowLimit       = MIN_SATURATION_VALUE;
    colorSaturationTransitionState->highLimit      = MAX_SATURATION_VALUE;

    SetHSVRemainingTime(endpoint);

    // kick off the state machine:
    scheduleTimerCallbackMs(configureHSVEventControl(endpoint), transitionTime ? TRANSITION_UPDATE_TIME_MS.count() : 0);

    return Status::Success;
}

/**
 * @brief Executes move Hue Command
 *
 * @param[in] endpoint
 * @param[in] moveMode
 * @param[in] rate
 * @param[in] optionsMask
 * @param[in] optionsOverride
 * @param[in] isEnhanced If True, function was called by EnhancedMoveHue command and rate is a uint16 value. If False function
 * was called by MoveHue command and rate is a uint8 value
 * @return true Success
 * @return false Failed
 */
bool ColorControlServer::moveHueCommand(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                        HueMoveMode moveMode, uint16_t rate, uint8_t optionsMask, uint8_t optionsOverride,
                                        bool isEnhanced)
{
    MATTER_TRACE_SCOPE("moveHue", "ColorControl");
    EndpointId endpoint                               = commandPath.mEndpointId;
    Status status                                     = Status::Success;
    ColorHueTransitionState * colorHueTransitionState = getColorHueTransitionState(endpoint);

    VerifyOrExit(colorHueTransitionState != nullptr, status = Status::UnsupportedEndpoint);

    // check moveMode before any operation is done on the transition states
    if (moveMode == HueMoveMode::kUnknownEnumValue || (rate == 0 && moveMode != HueMoveMode::kStop))
    {
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return true;
    }

    if (!shouldExecuteIfOff(endpoint, optionsMask, optionsOverride))
    {
        commandObj->AddStatus(commandPath, Status::Success);
        return true;
    }

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);
    // now, kick off the state machine.
    initHueTransitionState(endpoint, colorHueTransitionState, isEnhanced);

    if (moveMode == HueMoveMode::kStop)
    {
        // Per spec any saturation transition must also be cancelled.
        Color16uTransitionState * saturationState = getSaturationTransitionState(endpoint);
        initSaturationTransitionState(endpoint, saturationState);
        commandObj->AddStatus(commandPath, Status::Success);
        return true;
    }

    // Handle color mode transition, if necessary.
    if (isEnhanced)
    {
        handleModeSwitch(endpoint, ColorControl::EnhancedColorMode::kEnhancedCurrentHueAndCurrentSaturation);
    }
    else
    {
        handleModeSwitch(endpoint, ColorControl::EnhancedColorMode::kCurrentHueAndCurrentSaturation);
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
    colorHueTransitionState->endpoint       = endpoint;
    colorHueTransitionState->repeat         = true;

    // hue movement can last forever. Indicate this with a remaining time of maxint
    Attributes::RemainingTime::Set(endpoint, MAX_INT16U_VALUE);

    // kick off the state machine:
    scheduleTimerCallbackMs(configureHSVEventControl(endpoint), TRANSITION_UPDATE_TIME_MS.count());

exit:
    commandObj->AddStatus(commandPath, status);
    return true;
}

/**
 * @brief Executes move to hue command
 *
 * @param[in] endpoint
 * @param[in] hue
 * @param[in] hueMoveMode
 * @param[in] transitionTime
 * @param[in] optionsMask
 * @param[in] optionsOverride
 * @param[in] isEnhanced If True, function was called by EnhancedMoveHue command and rate is a uint16 value. If False function
 * was called by MoveHue command and rate is a uint8 value
 * @return true Success
 * @return false Failed
 */
bool ColorControlServer::moveToHueCommand(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                          uint16_t hue, HueDirection moveDirection, uint16_t transitionTime, uint8_t optionsMask,
                                          uint8_t optionsOverride, bool isEnhanced)
{
    MATTER_TRACE_SCOPE("moveToHue", "ColorControl");
    EndpointId endpoint = commandPath.mEndpointId;

    Status status       = Status::Success;
    uint16_t currentHue = 0;
    HueDirection direction;

    ColorHueTransitionState * colorHueTransitionState = getColorHueTransitionState(endpoint);

    VerifyOrExit(colorHueTransitionState != nullptr, status = Status::UnsupportedEndpoint);

    // Standard Hue limit checking:  hue is 0..254.  Spec dictates we ignore
    // this and report a constraint error.
    if (!isEnhanced && (hue > MAX_HUE_VALUE))
    {
        commandObj->AddStatus(commandPath, Status::ConstraintError);
        return true;
    }

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
    case HueDirection::kShortestDistance:
        if ((isEnhanced && (static_cast<uint16_t>(currentHue - hue) > HALF_MAX_UINT16T)) ||
            (!isEnhanced && (static_cast<uint8_t>(currentHue - hue) > HALF_MAX_UINT8T)))
        {
            direction = HueDirection::kUp;
        }
        else
        {
            direction = HueDirection::kDown;
        }
        break;
    case HueDirection::kLongestDistance:
        if ((isEnhanced && (static_cast<uint16_t>(currentHue - hue) > HALF_MAX_UINT16T)) ||
            (!isEnhanced && (static_cast<uint8_t>(currentHue - hue) > HALF_MAX_UINT8T)))
        {
            direction = HueDirection::kDown;
        }
        else
        {
            direction = HueDirection::kUp;
        }
        break;
    case HueDirection::kUp:
    case HueDirection::kDown:
        direction = moveDirection;
        break;
    case HueDirection::kUnknownEnumValue:
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return true;
        /* No default case, so if a new direction value gets added we will just fail
           to compile until we handle it correctly.  */
    }

    if (!shouldExecuteIfOff(endpoint, optionsMask, optionsOverride))
    {
        commandObj->AddStatus(commandPath, Status::Success);
        return true;
    }

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    // Handle color mode transition, if necessary.
    if (isEnhanced)
    {
        handleModeSwitch(endpoint, ColorControl::EnhancedColorMode::kEnhancedCurrentHueAndCurrentSaturation);
    }
    else
    {
        handleModeSwitch(endpoint, ColorControl::EnhancedColorMode::kCurrentHueAndCurrentSaturation);
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

    colorHueTransitionState->stepsRemaining = max<uint16_t>(transitionTime, 1);
    colorHueTransitionState->stepsTotal     = colorHueTransitionState->stepsRemaining;
    colorHueTransitionState->timeRemaining  = transitionTime;
    colorHueTransitionState->endpoint       = endpoint;
    colorHueTransitionState->up             = (direction == HueDirection::kUp);
    colorHueTransitionState->repeat         = false;

    SetHSVRemainingTime(endpoint);

    // kick off the state machine:
    scheduleTimerCallbackMs(configureHSVEventControl(endpoint), transitionTime ? TRANSITION_UPDATE_TIME_MS.count() : 0);

exit:
    commandObj->AddStatus(commandPath, status);
    return true;
}

/**
 * @brief executes move to hue and saturatioan command
 *
 * @param[in] commandObj
 * @param[in] commandPath
 * @param[in] hue
 * @param[in] saturation
 * @param[in] transitionTime
 * @param[in] optionsMask
 * @param[in] optionsOverride
 * @param[in] isEnhanced If True, function was called by EnhancedMoveHue command and rate is a uint16 value. If False function
 * was called by MoveHue command and rate is a uint8 value
 * @return true Success
 * @return false Failed
 */
bool ColorControlServer::moveToHueAndSaturationCommand(app::CommandHandler * commandObj,
                                                       const app::ConcreteCommandPath & commandPath, uint16_t hue,
                                                       uint8_t saturation, uint16_t transitionTime, uint8_t optionsMask,
                                                       uint8_t optionsOverride, bool isEnhanced)
{
    MATTER_TRACE_SCOPE("moveToHueAndSaturation", "ColorControl");
    // limit checking:  hue and saturation are 0..254.  Spec dictates we ignore
    // this and report a constraint error.
    if ((!isEnhanced && hue > MAX_HUE_VALUE) || saturation > MAX_SATURATION_VALUE)
    {
        commandObj->AddStatus(commandPath, Status::ConstraintError);
        return true;
    }

    if (!shouldExecuteIfOff(commandPath.mEndpointId, optionsMask, optionsOverride))
    {
        commandObj->AddStatus(commandPath, Status::Success);
        return true;
    }
    Status status = moveToHueAndSaturation(hue, saturation, transitionTime, isEnhanced, commandPath.mEndpointId);
#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
    ScenesManagement::ScenesServer::Instance().MakeSceneInvalidForAllFabrics(commandPath.mEndpointId);
#endif // MATTER_DM_PLUGIN_SCENES_MANAGEMENT
    commandObj->AddStatus(commandPath, status);
    return true;
}

/**
 * @brief Executes step hue command
 *
 * @param[in] endpoint
 * @param[in] stepMode
 * @param[in] stepSize
 * @param[in] transitionTime
 * @param[in] optionsMask
 * @param[in] optionsOverride
 * @param[in] isEnhanced If True, function was called by EnhancedMoveHue command and rate is a uint16 value. If False function
 * was called by MoveHue command and rate is a uint8 value
 * @return true Success
 * @return false Failed
 */
bool ColorControlServer::stepHueCommand(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                        HueStepMode stepMode, uint16_t stepSize, uint16_t transitionTime, uint8_t optionsMask,
                                        uint8_t optionsOverride, bool isEnhanced)
{
    MATTER_TRACE_SCOPE("stepHue", "ColorControl");
    EndpointId endpoint = commandPath.mEndpointId;

    Status status = Status::Success;

    ColorHueTransitionState * colorHueTransitionState = getColorHueTransitionState(endpoint);
    VerifyOrExit(colorHueTransitionState != nullptr, status = Status::UnsupportedEndpoint);

    // Confirm validity of the step mode received
    if (stepMode == HueStepMode::kUnknownEnumValue)
    {
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return true;
    }

    if (!shouldExecuteIfOff(endpoint, optionsMask, optionsOverride))
    {
        commandObj->AddStatus(commandPath, Status::Success);
        return true;
    }

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    // Handle color mode transition, if necessary.
    if (isEnhanced)
    {
        handleModeSwitch(endpoint, ColorControl::EnhancedColorMode::kEnhancedCurrentHueAndCurrentSaturation);
    }
    else
    {
        handleModeSwitch(endpoint, ColorControl::EnhancedColorMode::kCurrentHueAndCurrentSaturation);
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

    colorHueTransitionState->stepsRemaining = max<uint16_t>(transitionTime, 1);
    colorHueTransitionState->stepsTotal     = colorHueTransitionState->stepsRemaining;
    colorHueTransitionState->timeRemaining  = transitionTime;
    colorHueTransitionState->endpoint       = endpoint;
    colorHueTransitionState->repeat         = false;

    SetHSVRemainingTime(endpoint);

    // kick off the state machine:
    scheduleTimerCallbackMs(configureHSVEventControl(endpoint), transitionTime ? TRANSITION_UPDATE_TIME_MS.count() : 0);

exit:
    commandObj->AddStatus(commandPath, status);
    return true;
}

bool ColorControlServer::moveSaturationCommand(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                               const Commands::MoveSaturation::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("moveSaturation", "ColorControl");
    auto & moveMode        = commandData.moveMode;
    auto & rate            = commandData.rate;
    auto & optionsMask     = commandData.optionsMask;
    auto & optionsOverride = commandData.optionsOverride;
    EndpointId endpoint    = commandPath.mEndpointId;
    Status status          = Status::Success;

    Color16uTransitionState * colorSaturationTransitionState = getSaturationTransitionState(endpoint);
    VerifyOrExit(colorSaturationTransitionState != nullptr, status = Status::UnsupportedEndpoint);

    // check moveMode before any operation is done on the transition states
    if (moveMode == SaturationMoveMode::kUnknownEnumValue || (rate == 0 && moveMode != SaturationMoveMode::kStop))
    {
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return true;
    }

    if (!shouldExecuteIfOff(endpoint, optionsMask, optionsOverride))
    {
        commandObj->AddStatus(commandPath, Status::Success);
        return true;
    }

    uint16_t transitionTime;

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    // now, kick off the state machine.
    initSaturationTransitionState(endpoint, colorSaturationTransitionState);

    if (moveMode == SaturationMoveMode::kStop)
    {
        // Per spec any hue transition must also be cancelled.
        ColorHueTransitionState * hueState = getColorHueTransitionState(endpoint);
        initHueTransitionState(endpoint, hueState, false /*isEnhancedHue don't care*/);
        commandObj->AddStatus(commandPath, Status::Success);
        return true;
    }

    // Handle color mode transition, if necessary.
    handleModeSwitch(endpoint, ColorControl::EnhancedColorMode::kCurrentHueAndCurrentSaturation);

    if (moveMode == SaturationMoveMode::kUp)
    {
        colorSaturationTransitionState->finalValue = MAX_SATURATION_VALUE;
    }
    else if (moveMode == SaturationMoveMode::kDown)
    {
        colorSaturationTransitionState->finalValue = MIN_SATURATION_VALUE;
    }

    transitionTime = computeTransitionTimeFromStateAndRate(colorSaturationTransitionState, rate);

    colorSaturationTransitionState->stepsRemaining = transitionTime;
    colorSaturationTransitionState->stepsTotal     = transitionTime;
    colorSaturationTransitionState->timeRemaining  = transitionTime;
    colorSaturationTransitionState->endpoint       = endpoint;
    colorSaturationTransitionState->lowLimit       = MIN_SATURATION_VALUE;
    colorSaturationTransitionState->highLimit      = MAX_SATURATION_VALUE;

    SetHSVRemainingTime(endpoint);

    // kick off the state machine:
    scheduleTimerCallbackMs(configureHSVEventControl(endpoint), TRANSITION_UPDATE_TIME_MS.count());

exit:
    commandObj->AddStatus(commandPath, status);
    return true;
}

/**
 * @brief executes move to saturation command
 *
 * @param commandObj
 * @param commandPath
 * @param commandData
 * @return true
 * @return false
 */
bool ColorControlServer::moveToSaturationCommand(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                 const Commands::MoveToSaturation::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("moveToSaturation", "ColorControl");
    // limit checking: saturation is 0..254.  Spec dictates we ignore
    // this and report a malformed packet.
    if (commandData.saturation > MAX_SATURATION_VALUE)
    {
        commandObj->AddStatus(commandPath, Status::ConstraintError);
        return true;
    }

    if (!shouldExecuteIfOff(commandPath.mEndpointId, commandData.optionsMask, commandData.optionsOverride))
    {
        commandObj->AddStatus(commandPath, Status::Success);
        return true;
    }
    Status status = moveToSaturation(commandData.saturation, commandData.transitionTime, commandPath.mEndpointId);
#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
    ScenesManagement::ScenesServer::Instance().MakeSceneInvalidForAllFabrics(commandPath.mEndpointId);
#endif // MATTER_DM_PLUGIN_SCENES_MANAGEMENT
    commandObj->AddStatus(commandPath, status);
    return true;
}

bool ColorControlServer::stepSaturationCommand(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                               const Commands::StepSaturation::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("stepSaturation", "ColorControl");
    auto stepMode             = commandData.stepMode;
    uint8_t stepSize          = commandData.stepSize;
    uint8_t transitionTime    = commandData.transitionTime;
    uint8_t optionsMask       = commandData.optionsMask;
    uint8_t optionsOverride   = commandData.optionsOverride;
    EndpointId endpoint       = commandPath.mEndpointId;
    Status status             = Status::Success;
    uint8_t currentSaturation = 0;

    Color16uTransitionState * colorSaturationTransitionState = getSaturationTransitionState(endpoint);
    VerifyOrExit(colorSaturationTransitionState != nullptr, status = Status::UnsupportedEndpoint);

    // Confirm validity of the step mode received
    if (stepMode == SaturationStepMode::kUnknownEnumValue)
    {
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return true;
    }

    if (!shouldExecuteIfOff(endpoint, optionsMask, optionsOverride))
    {
        commandObj->AddStatus(commandPath, Status::Success);
        return true;
    }

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    // Handle color mode transition, if necessary.
    handleModeSwitch(endpoint, ColorControl::EnhancedColorMode::kCurrentHueAndCurrentSaturation);

    // now, kick off the state machine.
    initSaturationTransitionState(endpoint, colorSaturationTransitionState);
    currentSaturation = static_cast<uint8_t>(colorSaturationTransitionState->currentValue);

    if (stepMode == SaturationStepMode::kUp)
    {
        colorSaturationTransitionState->finalValue = addSaturation(currentSaturation, stepSize);
    }
    else if (stepMode == SaturationStepMode::kDown)
    {
        colorSaturationTransitionState->finalValue = subtractSaturation(currentSaturation, stepSize);
    }
    colorSaturationTransitionState->stepsRemaining = max<uint8_t>(transitionTime, 1);
    colorSaturationTransitionState->stepsTotal     = colorSaturationTransitionState->stepsRemaining;
    colorSaturationTransitionState->timeRemaining  = transitionTime;
    colorSaturationTransitionState->endpoint       = endpoint;
    colorSaturationTransitionState->lowLimit       = MIN_SATURATION_VALUE;
    colorSaturationTransitionState->highLimit      = MAX_SATURATION_VALUE;

    SetHSVRemainingTime(endpoint);

    // kick off the state machine:
    scheduleTimerCallbackMs(configureHSVEventControl(endpoint), transitionTime ? TRANSITION_UPDATE_TIME_MS.count() : 0);

exit:
    commandObj->AddStatus(commandPath, status);
    return true;
}

bool ColorControlServer::colorLoopCommand(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                          const Commands::ColorLoopSet::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("colorLoop", "ColorControl");
    auto updateFlags          = commandData.updateFlags;
    auto action               = commandData.action;
    auto direction            = commandData.direction;
    uint16_t time             = commandData.time;
    uint16_t startHue         = commandData.startHue;
    uint8_t optionsMask       = commandData.optionsMask;
    uint8_t optionsOverride   = commandData.optionsOverride;
    EndpointId endpoint       = commandPath.mEndpointId;
    Status status             = Status::Success;
    uint8_t isColorLoopActive = 0;
    uint8_t deactiveColorLoop = 0;

    ColorHueTransitionState * colorHueTransitionState = getColorHueTransitionState(endpoint);
    VerifyOrExit(colorHueTransitionState != nullptr, status = Status::UnsupportedEndpoint);

    // Validate the action and direction parameters of the command
    if (action == ColorLoopAction::kUnknownEnumValue || direction == ColorLoopDirection::kUnknownEnumValue)
    {
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return true;
    }

    if (!shouldExecuteIfOff(endpoint, optionsMask, optionsOverride))
    {
        commandObj->AddStatus(commandPath, Status::Success);
        return true;
    }

    Attributes::ColorLoopActive::Get(endpoint, &isColorLoopActive);

    deactiveColorLoop = updateFlags.Has(ColorLoopUpdateFlags::kUpdateAction) && (action == ColorLoopAction::kDeactivate);

    if (updateFlags.Has(ColorLoopUpdateFlags::kUpdateDirection))
    {
        Attributes::ColorLoopDirection::Set(endpoint, to_underlying(direction));

        // Checks if color loop is active and stays active
        if (isColorLoopActive && !deactiveColorLoop)
        {
            colorHueTransitionState->up                 = (direction == ColorLoopDirection::kIncrementHue);
            colorHueTransitionState->initialEnhancedHue = colorHueTransitionState->currentEnhancedHue;

            if (direction == ColorLoopDirection::kIncrementHue)
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

    if (updateFlags.Has(ColorLoopUpdateFlags::kUpdateTime))
    {
        Attributes::ColorLoopTime::Set(endpoint, time);

        // Checks if color loop is active and stays active
        if (isColorLoopActive && !deactiveColorLoop)
        {
            colorHueTransitionState->stepsTotal         = static_cast<uint16_t>(time * TRANSITION_STEPS_PER_1S);
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

    if (updateFlags.Has(ColorLoopUpdateFlags::kUpdateStartHue))
    {
        Attributes::ColorLoopStartEnhancedHue::Set(endpoint, startHue);
    }

    if (updateFlags.Has(ColorLoopUpdateFlags::kUpdateAction))
    {
        if (action == ColorLoopAction::kDeactivate)
        {
            if (isColorLoopActive)
            {
                stopAllColorTransitions(endpoint);

                Attributes::ColorLoopActive::Set(endpoint, false);

                uint16_t storedEnhancedHue = 0;
                Attributes::ColorLoopStoredEnhancedHue::Get(endpoint, &storedEnhancedHue);
                Attributes::EnhancedCurrentHue::Set(endpoint, storedEnhancedHue);
            }
            else
            {
                // Do Nothing since it's not on
            }
        }
        else if (action == ColorLoopAction::kActivateFromColorLoopStartEnhancedHue)
        {
            startColorLoop(endpoint, true);
        }
        else if (action == ColorLoopAction::kActivateFromEnhancedCurrentHue)
        {
            startColorLoop(endpoint, false);
        }
    }

exit:
#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
    ScenesManagement::ScenesServer::Instance().MakeSceneInvalidForAllFabrics(endpoint);
#endif // MATTER_DM_PLUGIN_SCENES_MANAGEMENT
    commandObj->AddStatus(commandPath, status);
    return true;
}

/**
 * @brief updates Hue and saturation after timer is finished
 *
 * @param endpoint
 */
void ColorControlServer::updateHueSatCommand(EndpointId endpoint)
{
    MATTER_TRACE_SCOPE("updateHueSat", "ColorControl");
    ColorHueTransitionState * colorHueTransitionState        = getColorHueTransitionState(endpoint);
    Color16uTransitionState * colorSaturationTransitionState = getSaturationTransitionState(endpoint);

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

    if (colorHueTransitionState->isEnhancedHue)
    {
        if (previousEnhancedhue != colorHueTransitionState->currentEnhancedHue)
        {
            Attributes::EnhancedCurrentHue::Set(endpoint, colorHueTransitionState->currentEnhancedHue);
            Attributes::CurrentHue::Set(endpoint, static_cast<uint8_t>(colorHueTransitionState->currentEnhancedHue >> 8));

            ChipLogProgress(Zcl, "Enhanced Hue %d endpoint %d", colorHueTransitionState->currentEnhancedHue, endpoint);
        }
    }
    else
    {
        if (previousHue != colorHueTransitionState->currentHue)
        {
            Attributes::CurrentHue::Set(colorHueTransitionState->endpoint, colorHueTransitionState->currentHue);
            ChipLogProgress(Zcl, "Hue %d endpoint %d", colorHueTransitionState->currentHue, endpoint);
        }
    }

    if (previousSaturation != colorSaturationTransitionState->currentValue)
    {
        Attributes::CurrentSaturation::Set(colorSaturationTransitionState->endpoint,
                                           (uint8_t) colorSaturationTransitionState->currentValue);
        ChipLogProgress(Zcl, "Saturation %d endpoint %d", colorSaturationTransitionState->currentValue, endpoint);
    }

    computePwmFromHsv(endpoint);
}

#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_XY

/**
 * @brief Returns Color16uTransitionState for X color associated to an endpoint
 *
 * @param endpoint
 * @return ColorControlServer::Color16uTransitionState*
 */
ColorControlServer::Color16uTransitionState * ColorControlServer::getXTransitionState(EndpointId endpoint)
{
    uint16_t index =
        emberAfGetClusterServerEndpointIndex(endpoint, ColorControl::Id, MATTER_DM_COLOR_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);

    Color16uTransitionState * state = nullptr;
    if (index < ArraySize(colorXtransitionStates))
    {
        state = &colorXtransitionStates[index];
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
    uint16_t index =
        emberAfGetClusterServerEndpointIndex(endpoint, ColorControl::Id, MATTER_DM_COLOR_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);

    Color16uTransitionState * state = nullptr;
    if (index < ArraySize(colorYtransitionStates))
    {
        state = &colorYtransitionStates[index];
    }

    return state;
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
 * @param colorX target X
 * @param colorY target Y
 * @param transitionTime transition time in 10th of seconds
 * @param endpoint target endpoint where to execute move
 * @return Status::Success if successful,Status::UnsupportedEndpoint XY is not supported on the endpoint
 */
Status ColorControlServer::moveToColor(uint16_t colorX, uint16_t colorY, uint16_t transitionTime, EndpointId endpoint)
{
    Color16uTransitionState * colorXTransitionState = getXTransitionState(endpoint);
    Color16uTransitionState * colorYTransitionState = getYTransitionState(endpoint);

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
    colorXTransitionState->stepsRemaining = max<uint16_t>(transitionTime, 1);
    colorXTransitionState->stepsTotal     = colorXTransitionState->stepsRemaining;
    colorXTransitionState->timeRemaining  = transitionTime;
    colorXTransitionState->endpoint       = endpoint;
    colorXTransitionState->lowLimit       = MIN_CIE_XY_VALUE;
    colorXTransitionState->highLimit      = MAX_CIE_XY_VALUE;

    Attributes::CurrentY::Get(endpoint, &(colorYTransitionState->initialValue));
    Attributes::CurrentY::Get(endpoint, &(colorYTransitionState->currentValue));
    colorYTransitionState->finalValue     = colorY;
    colorYTransitionState->stepsRemaining = colorXTransitionState->stepsRemaining;
    colorYTransitionState->stepsTotal     = colorXTransitionState->stepsRemaining;
    colorYTransitionState->timeRemaining  = transitionTime;
    colorYTransitionState->endpoint       = endpoint;
    colorYTransitionState->lowLimit       = MIN_CIE_XY_VALUE;
    colorYTransitionState->highLimit      = MAX_CIE_XY_VALUE;

    Attributes::RemainingTime::Set(endpoint, transitionTime);

    // kick off the state machine:
    scheduleTimerCallbackMs(configureXYEventControl(endpoint), transitionTime ? TRANSITION_UPDATE_TIME_MS.count() : 0);

    return Status::Success;
}

bool ColorControlServer::moveToColorCommand(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                            const Commands::MoveToColor::DecodableType & commandData)
{
    if (!shouldExecuteIfOff(commandPath.mEndpointId, commandData.optionsMask, commandData.optionsOverride))
    {
        commandObj->AddStatus(commandPath, Status::Success);
        return true;
    }

    Status status = moveToColor(commandData.colorX, commandData.colorY, commandData.transitionTime, commandPath.mEndpointId);
#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
    ScenesManagement::ScenesServer::Instance().MakeSceneInvalidForAllFabrics(commandPath.mEndpointId);
#endif // MATTER_DM_PLUGIN_SCENES_MANAGEMENT
    commandObj->AddStatus(commandPath, status);
    return true;
}

bool ColorControlServer::moveColorCommand(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                          const Commands::MoveColor::DecodableType & commandData)
{
    int16_t rateX           = commandData.rateX;
    int16_t rateY           = commandData.rateY;
    uint8_t optionsMask     = commandData.optionsMask;
    uint8_t optionsOverride = commandData.optionsOverride;
    EndpointId endpoint     = commandPath.mEndpointId;
    Status status           = Status::Success;

    Color16uTransitionState * colorXTransitionState = getXTransitionState(endpoint);
    Color16uTransitionState * colorYTransitionState = getYTransitionState(endpoint);

    VerifyOrExit(colorXTransitionState != nullptr, status = Status::UnsupportedEndpoint);
    VerifyOrExit(colorYTransitionState != nullptr, status = Status::UnsupportedEndpoint);

    if (!shouldExecuteIfOff(endpoint, optionsMask, optionsOverride))
    {
        commandObj->AddStatus(commandPath, Status::Success);
        return true;
    }

    uint16_t transitionTimeX, transitionTimeY;
    uint16_t unsignedRate;

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    if (rateX == 0 && rateY == 0)
    {
        commandObj->AddStatus(commandPath, Status::Success);
        return true;
    }

    // Handle color mode transition, if necessary.
    handleModeSwitch(endpoint, ColorControl::EnhancedColorMode::kCurrentXAndCurrentY);

    // now, kick off the state machine.
    Attributes::CurrentX::Get(endpoint, &(colorXTransitionState->initialValue));
    colorXTransitionState->currentValue = colorXTransitionState->initialValue;
    if (rateX > 0)
    {
        colorXTransitionState->finalValue = MAX_CIE_XY_VALUE;
        unsignedRate                      = static_cast<uint16_t>(rateX);
    }
    else
    {
        colorXTransitionState->finalValue = MIN_CIE_XY_VALUE;
        unsignedRate                      = static_cast<uint16_t>(rateX * -1);
    }
    transitionTimeX                       = computeTransitionTimeFromStateAndRate(colorXTransitionState, unsignedRate);
    colorXTransitionState->stepsRemaining = transitionTimeX;
    colorXTransitionState->stepsTotal     = transitionTimeX;
    colorXTransitionState->timeRemaining  = transitionTimeX;
    colorXTransitionState->endpoint       = endpoint;
    colorXTransitionState->lowLimit       = MIN_CIE_XY_VALUE;
    colorXTransitionState->highLimit      = MAX_CIE_XY_VALUE;

    Attributes::CurrentY::Get(endpoint, &(colorYTransitionState->initialValue));
    colorYTransitionState->currentValue = colorYTransitionState->initialValue;
    if (rateY > 0)
    {
        colorYTransitionState->finalValue = MAX_CIE_XY_VALUE;
        unsignedRate                      = static_cast<uint16_t>(rateY);
    }
    else
    {
        colorYTransitionState->finalValue = MIN_CIE_XY_VALUE;
        unsignedRate                      = static_cast<uint16_t>(rateY * -1);
    }
    transitionTimeY                       = computeTransitionTimeFromStateAndRate(colorYTransitionState, unsignedRate);
    colorYTransitionState->stepsRemaining = transitionTimeY;
    colorYTransitionState->stepsTotal     = transitionTimeY;
    colorYTransitionState->timeRemaining  = transitionTimeY;
    colorYTransitionState->endpoint       = endpoint;
    colorYTransitionState->lowLimit       = MIN_CIE_XY_VALUE;
    colorYTransitionState->highLimit      = MAX_CIE_XY_VALUE;

    if (transitionTimeX < transitionTimeY)
    {
        Attributes::RemainingTime::Set(endpoint, transitionTimeX);
    }
    else
    {
        Attributes::RemainingTime::Set(endpoint, transitionTimeY);
    }

    // kick off the state machine:
    scheduleTimerCallbackMs(configureXYEventControl(endpoint), TRANSITION_UPDATE_TIME_MS.count());

exit:
    commandObj->AddStatus(commandPath, status);
    return true;
}

bool ColorControlServer::stepColorCommand(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                          const Commands::StepColor::DecodableType & commandData)
{
    int16_t stepX           = commandData.stepX;
    int16_t stepY           = commandData.stepY;
    uint16_t transitionTime = commandData.transitionTime;
    uint8_t optionsMask     = commandData.optionsMask;
    uint8_t optionsOverride = commandData.optionsOverride;
    EndpointId endpoint     = commandPath.mEndpointId;
    uint16_t currentColorX  = 0;
    uint16_t currentColorY  = 0;
    uint16_t colorX         = 0;
    uint16_t colorY         = 0;

    Status status = Status::Success;

    Color16uTransitionState * colorXTransitionState = getXTransitionState(endpoint);
    Color16uTransitionState * colorYTransitionState = getYTransitionState(endpoint);

    VerifyOrExit(colorXTransitionState != nullptr, status = Status::UnsupportedEndpoint);
    VerifyOrExit(colorYTransitionState != nullptr, status = Status::UnsupportedEndpoint);

    if (!shouldExecuteIfOff(endpoint, optionsMask, optionsOverride))
    {
        commandObj->AddStatus(commandPath, Status::Success);
        return true;
    }

    Attributes::CurrentX::Get(endpoint, &currentColorX);
    Attributes::CurrentY::Get(endpoint, &currentColorY);

    colorX = findNewColorValueFromStep(currentColorX, stepX);
    colorY = findNewColorValueFromStep(currentColorY, stepY);

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    // Handle color mode transition, if necessary.
    handleModeSwitch(endpoint, ColorControl::EnhancedColorMode::kCurrentXAndCurrentY);

    // now, kick off the state machine.
    colorXTransitionState->initialValue   = currentColorX;
    colorXTransitionState->currentValue   = currentColorX;
    colorXTransitionState->finalValue     = colorX;
    colorXTransitionState->stepsRemaining = max<uint16_t>(transitionTime, 1);
    colorXTransitionState->stepsTotal     = colorXTransitionState->stepsRemaining;
    colorXTransitionState->timeRemaining  = transitionTime;
    colorXTransitionState->endpoint       = endpoint;
    colorXTransitionState->lowLimit       = MIN_CIE_XY_VALUE;
    colorXTransitionState->highLimit      = MAX_CIE_XY_VALUE;

    colorYTransitionState->initialValue   = currentColorY;
    colorYTransitionState->currentValue   = currentColorY;
    colorYTransitionState->finalValue     = colorY;
    colorYTransitionState->stepsRemaining = colorXTransitionState->stepsRemaining;
    colorYTransitionState->stepsTotal     = colorXTransitionState->stepsRemaining;
    colorYTransitionState->timeRemaining  = transitionTime;
    colorYTransitionState->endpoint       = endpoint;
    colorYTransitionState->lowLimit       = MIN_CIE_XY_VALUE;
    colorYTransitionState->highLimit      = MAX_CIE_XY_VALUE;

    Attributes::RemainingTime::Set(endpoint, transitionTime);

    // kick off the state machine:
    scheduleTimerCallbackMs(configureXYEventControl(endpoint), transitionTime ? TRANSITION_UPDATE_TIME_MS.count() : 0);

exit:
    commandObj->AddStatus(commandPath, status);
    return true;
}

/**
 * @brief Update XY color after timer is finished
 *
 * @param endpoint
 */
void ColorControlServer::updateXYCommand(EndpointId endpoint)
{
    Color16uTransitionState * colorXTransitionState = getXTransitionState(endpoint);
    Color16uTransitionState * colorYTransitionState = getYTransitionState(endpoint);
    bool isXTransitionDone, isYTransitionDone;

    // compute new values for X and Y.
    isXTransitionDone = computeNewColor16uValue(colorXTransitionState);
    isYTransitionDone = computeNewColor16uValue(colorYTransitionState);

    Attributes::RemainingTime::Set(endpoint, max(colorXTransitionState->timeRemaining, colorYTransitionState->timeRemaining));

    if (isXTransitionDone && isYTransitionDone)
    {
        stopAllColorTransitions(endpoint);
    }
    else
    {
        scheduleTimerCallbackMs(configureXYEventControl(endpoint), TRANSITION_UPDATE_TIME_MS.count());
    }

    // update the attributes
    Attributes::CurrentX::Set(endpoint, colorXTransitionState->currentValue);
    Attributes::CurrentY::Set(endpoint, colorYTransitionState->currentValue);

    ChipLogProgress(Zcl, "Color X %d Color Y %d", colorXTransitionState->currentValue, colorYTransitionState->currentValue);

    computePwmFromXy(endpoint);
}

#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_XY

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP
/**
 * @brief Get the Temp Transition State object associated to the endpoint
 *
 * @param endpoint
 * @return Color16uTransitionState*
 */
ColorControlServer::Color16uTransitionState * ColorControlServer::getTempTransitionState(EndpointId endpoint)
{
    uint16_t index =
        emberAfGetClusterServerEndpointIndex(endpoint, ColorControl::Id, MATTER_DM_COLOR_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);

    Color16uTransitionState * state = nullptr;
    if (index < ArraySize(colorTempTransitionStates))
    {
        state = &colorTempTransitionStates[index];
    }

    return state;
}

/**
 * @brief executes move to color temp logic
 *
 * @param aEndpoint
 * @param colorTemperature
 * @param transitionTime
 * @return Status::Success if successful, Status::UnsupportedEndpoint if the endpoint doesn't support color temperature
 */
Status ColorControlServer::moveToColorTemp(EndpointId aEndpoint, uint16_t colorTemperature, uint16_t transitionTime)
{
    EndpointId endpoint = aEndpoint;

    Color16uTransitionState * colorTempTransitionState = getTempTransitionState(endpoint);
    VerifyOrReturnError(nullptr != colorTempTransitionState, Status::UnsupportedEndpoint);

    uint16_t temperatureMin = MIN_TEMPERATURE_VALUE;
    Attributes::ColorTempPhysicalMinMireds::Get(endpoint, &temperatureMin);

    uint16_t temperatureMax = MAX_TEMPERATURE_VALUE;
    Attributes::ColorTempPhysicalMaxMireds::Get(endpoint, &temperatureMax);

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    // Handle color mode transition, if necessary.
    handleModeSwitch(endpoint, ColorControl::EnhancedColorMode::kColorTemperature);

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
    colorTempTransitionState->stepsRemaining = max<uint16_t>(transitionTime, 1);
    colorTempTransitionState->stepsTotal     = colorTempTransitionState->stepsRemaining;
    colorTempTransitionState->timeRemaining  = transitionTime;
    colorTempTransitionState->endpoint       = endpoint;
    colorTempTransitionState->lowLimit       = temperatureMin;
    colorTempTransitionState->highLimit      = temperatureMax;

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
    // 0x0000-0xffef        Set the ColorTemperatureMireds attribute to this value.
    // null                 Set the ColorTemperatureMireds attribute to its previous value.

    // Initialize startUpColorTempMireds to "maintain previous value" value null
    app::DataModel::Nullable<uint16_t> startUpColorTemp;
    Status status = Attributes::StartUpColorTemperatureMireds::Get(endpoint, startUpColorTemp);

    if (status == Status::Success && !startUpColorTemp.IsNull())
    {
        uint16_t updatedColorTemp = MAX_TEMPERATURE_VALUE;
        status                    = Attributes::ColorTemperatureMireds::Get(endpoint, &updatedColorTemp);

        if (status == Status::Success)
        {
            uint16_t tempPhysicalMin = MIN_TEMPERATURE_VALUE;
            Attributes::ColorTempPhysicalMinMireds::Get(endpoint, &tempPhysicalMin);

            uint16_t tempPhysicalMax = MAX_TEMPERATURE_VALUE;
            Attributes::ColorTempPhysicalMaxMireds::Get(endpoint, &tempPhysicalMax);

            if (tempPhysicalMin <= startUpColorTemp.Value() && startUpColorTemp.Value() <= tempPhysicalMax)
            {
                // Apply valid startup color temp value that is within physical limits of device.
                // Otherwise, the startup value is outside the device's supported range, and the
                // existing setting of ColorTemp attribute will be left unchanged (i.e., treated as
                // if startup color temp was set to null).
                updatedColorTemp = startUpColorTemp.Value();
                status           = Attributes::ColorTemperatureMireds::Set(endpoint, updatedColorTemp);

                if (status == Status::Success)
                {
                    // Set ColorMode attributes to reflect ColorTemperature.
                    uint8_t updateColorMode = ColorControl::EnhancedColorMode::kColorTemperature;
                    Attributes::ColorMode::Set(endpoint, updateColorMode);

                    updateColorMode = ColorControl::EnhancedColorMode::kColorTemperature;
                    Attributes::EnhancedColorMode::Set(endpoint, updateColorMode);
                }
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
    Color16uTransitionState * colorTempTransitionState = getTempTransitionState(endpoint);
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

    Attributes::RemainingTime::Set(endpoint, colorTempTransitionState->timeRemaining);

    if (isColorTempTransitionDone)
    {
        stopAllColorTransitions(endpoint);
    }
    else
    {
        scheduleTimerCallbackMs(configureTempEventControl(endpoint), TRANSITION_UPDATE_TIME_MS.count());
    }

    Attributes::ColorTemperatureMireds::Set(endpoint, colorTempTransitionState->currentValue);

    ChipLogProgress(Zcl, "Color Temperature %d", colorTempTransitionState->currentValue);

    computePwmFromTemp(endpoint);
}

/**
 * @brief move color temp command
 *
 * @param moveMode
 * @param rate
 * @param colorTemperatureMinimum
 * @param colorTemperatureMaximum
 * @param optionsMask
 * @param optionsOverride
 * @return true
 * @return false
 */
bool ColorControlServer::moveColorTempCommand(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                              const Commands::MoveColorTemperature::DecodableType & commandData)
{
    auto moveMode                    = commandData.moveMode;
    uint16_t rate                    = commandData.rate;
    uint16_t colorTemperatureMinimum = commandData.colorTemperatureMinimumMireds;
    uint16_t colorTemperatureMaximum = commandData.colorTemperatureMaximumMireds;
    uint8_t optionsMask              = commandData.optionsMask;
    uint8_t optionsOverride          = commandData.optionsOverride;
    EndpointId endpoint              = commandPath.mEndpointId;
    Status status                    = Status::Success;
    uint16_t tempPhysicalMin         = MIN_TEMPERATURE_VALUE;
    uint16_t tempPhysicalMax         = MAX_TEMPERATURE_VALUE;
    uint16_t transitionTime;

    Color16uTransitionState * colorTempTransitionState = getTempTransitionState(endpoint);
    VerifyOrExit(colorTempTransitionState != nullptr, status = Status::UnsupportedEndpoint);

    // check moveMode before any operation is done on the transition states
    if (moveMode == HueMoveMode::kUnknownEnumValue || (rate == 0 && moveMode != HueMoveMode::kStop))
    {
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return true;
    }

    if (!shouldExecuteIfOff(endpoint, optionsMask, optionsOverride))
    {
        commandObj->AddStatus(commandPath, Status::Success);
        return true;
    }

    Attributes::ColorTempPhysicalMinMireds::Get(endpoint, &tempPhysicalMin);
    Attributes::ColorTempPhysicalMaxMireds::Get(endpoint, &tempPhysicalMax);

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    if (moveMode == HueMoveMode::kStop)
    {
        commandObj->AddStatus(commandPath, Status::Success);
        return true;
    }

    if (colorTemperatureMinimum < tempPhysicalMin)
    {
        colorTemperatureMinimum = tempPhysicalMin;
    }
    if (colorTemperatureMaximum > tempPhysicalMax)
    {
        colorTemperatureMaximum = tempPhysicalMax;
    }

    // Handle color mode transition, if necessary.
    handleModeSwitch(endpoint, ColorControl::EnhancedColorMode::kColorTemperature);

    // now, kick off the state machine.
    colorTempTransitionState->initialValue = 0;
    Attributes::ColorTemperatureMireds::Get(endpoint, &colorTempTransitionState->initialValue);
    colorTempTransitionState->currentValue = colorTempTransitionState->initialValue;

    if (moveMode == HueMoveMode::kUp)
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
    transitionTime                           = computeTransitionTimeFromStateAndRate(colorTempTransitionState, rate);
    colorTempTransitionState->stepsRemaining = transitionTime;
    colorTempTransitionState->stepsTotal     = transitionTime;
    colorTempTransitionState->timeRemaining  = transitionTime;
    colorTempTransitionState->endpoint       = endpoint;
    colorTempTransitionState->lowLimit       = colorTemperatureMinimum;
    colorTempTransitionState->highLimit      = colorTemperatureMaximum;

    Attributes::RemainingTime::Set(endpoint, transitionTime);

    // kick off the state machine:
    scheduleTimerCallbackMs(configureTempEventControl(endpoint), TRANSITION_UPDATE_TIME_MS.count());

exit:
    commandObj->AddStatus(commandPath, status);
    return true;
}

bool ColorControlServer::moveToColorTempCommand(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                const Commands::MoveToColorTemperature::DecodableType & commandData)
{
    if (!shouldExecuteIfOff(commandPath.mEndpointId, commandData.optionsMask, commandData.optionsOverride))
    {
        commandObj->AddStatus(commandPath, Status::Success);
        return true;
    }

    Status status = moveToColorTemp(commandPath.mEndpointId, commandData.colorTemperatureMireds, commandData.transitionTime);
#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
    ScenesManagement::ScenesServer::Instance().MakeSceneInvalidForAllFabrics(commandPath.mEndpointId);
#endif // MATTER_DM_PLUGIN_SCENES_MANAGEMENT
    commandObj->AddStatus(commandPath, status);
    return true;
}

bool ColorControlServer::stepColorTempCommand(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                              const Commands::StepColorTemperature::DecodableType & commandData)
{
    auto stepMode                    = commandData.stepMode;
    uint16_t stepSize                = commandData.stepSize;
    uint16_t transitionTime          = commandData.transitionTime;
    uint16_t colorTemperatureMinimum = commandData.colorTemperatureMinimumMireds;
    uint16_t colorTemperatureMaximum = commandData.colorTemperatureMaximumMireds;
    uint8_t optionsMask              = commandData.optionsMask;
    uint8_t optionsOverride          = commandData.optionsOverride;
    EndpointId endpoint              = commandPath.mEndpointId;
    Status status                    = Status::Success;
    uint16_t tempPhysicalMin         = MIN_TEMPERATURE_VALUE;
    uint16_t tempPhysicalMax         = MAX_TEMPERATURE_VALUE;

    Color16uTransitionState * colorTempTransitionState = getTempTransitionState(endpoint);
    VerifyOrExit(colorTempTransitionState != nullptr, status = Status::UnsupportedEndpoint);

    // Confirm validity of the step mode received
    if (stepMode == HueStepMode::kUnknownEnumValue)
    {
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return true;
    }

    if (!shouldExecuteIfOff(endpoint, optionsMask, optionsOverride))
    {
        commandObj->AddStatus(commandPath, Status::Success);
        return true;
    }

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    Attributes::ColorTempPhysicalMinMireds::Get(endpoint, &tempPhysicalMin);
    Attributes::ColorTempPhysicalMaxMireds::Get(endpoint, &tempPhysicalMax);

    if (colorTemperatureMinimum < tempPhysicalMin)
    {
        colorTemperatureMinimum = tempPhysicalMin;
    }
    if (colorTemperatureMaximum > tempPhysicalMax)
    {
        colorTemperatureMaximum = tempPhysicalMax;
    }

    // Handle color mode transition, if necessary.
    handleModeSwitch(endpoint, ColorControl::EnhancedColorMode::kColorTemperature);

    // now, kick off the state machine.
    colorTempTransitionState->initialValue = 0;
    Attributes::ColorTemperatureMireds::Get(endpoint, &colorTempTransitionState->initialValue);
    colorTempTransitionState->currentValue = colorTempTransitionState->initialValue;

    if (stepMode == HueStepMode::kUp)
    {
        uint32_t finalValue32u = static_cast<uint32_t>(colorTempTransitionState->initialValue) + static_cast<uint32_t>(stepSize);
        if (finalValue32u > UINT16_MAX)
        {
            colorTempTransitionState->finalValue = UINT16_MAX;
        }
        else
        {
            colorTempTransitionState->finalValue = static_cast<uint16_t>(finalValue32u);
        }
    }
    else if (stepMode == HueStepMode::kDown)
    {
        uint32_t finalValue32u = static_cast<uint32_t>(colorTempTransitionState->initialValue) - static_cast<uint32_t>(stepSize);
        if (finalValue32u > UINT16_MAX)
        {
            colorTempTransitionState->finalValue = 0;
        }
        else
        {
            colorTempTransitionState->finalValue = static_cast<uint16_t>(finalValue32u);
        }
    }
    colorTempTransitionState->stepsRemaining = max<uint16_t>(transitionTime, 1);
    colorTempTransitionState->stepsTotal     = colorTempTransitionState->stepsRemaining;
    colorTempTransitionState->timeRemaining  = transitionTime;
    colorTempTransitionState->endpoint       = endpoint;
    colorTempTransitionState->lowLimit       = colorTemperatureMinimum;
    colorTempTransitionState->highLimit      = colorTemperatureMaximum;

    Attributes::RemainingTime::Set(endpoint, transitionTime);

    // kick off the state machine:
    scheduleTimerCallbackMs(configureTempEventControl(endpoint), transitionTime ? TRANSITION_UPDATE_TIME_MS.count() : 0);

exit:
    commandObj->AddStatus(commandPath, status);
    return true;
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

    uint8_t colorMode = 0;
    Attributes::ColorMode::Get(endpoint, &colorMode);

    if (colorMode == ColorControl::EnhancedColorMode::kColorTemperature)
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

/**********************************************************
 * Callbacks Implementation
 *********************************************************/

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV

bool emberAfColorControlClusterMoveHueCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                               const Commands::MoveHue::DecodableType & commandData)
{
    return ColorControlServer::Instance().moveHueCommand(commandObj, commandPath, commandData.moveMode, commandData.rate,
                                                         commandData.optionsMask, commandData.optionsOverride, false);
}

bool emberAfColorControlClusterMoveSaturationCallback(app::CommandHandler * commandObj,
                                                      const app::ConcreteCommandPath & commandPath,
                                                      const Commands::MoveSaturation::DecodableType & commandData)
{

    return ColorControlServer::Instance().moveSaturationCommand(commandObj, commandPath, commandData);
}

bool emberAfColorControlClusterMoveToHueCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                 const Commands::MoveToHue::DecodableType & commandData)
{
    return ColorControlServer::Instance().moveToHueCommand(commandObj, commandPath, commandData.hue, commandData.direction,
                                                           commandData.transitionTime, commandData.optionsMask,
                                                           commandData.optionsOverride, false);
}

bool emberAfColorControlClusterMoveToSaturationCallback(app::CommandHandler * commandObj,
                                                        const app::ConcreteCommandPath & commandPath,
                                                        const Commands::MoveToSaturation::DecodableType & commandData)
{
    return ColorControlServer::Instance().moveToSaturationCommand(commandObj, commandPath, commandData);
}

bool emberAfColorControlClusterMoveToHueAndSaturationCallback(app::CommandHandler * commandObj,
                                                              const app::ConcreteCommandPath & commandPath,
                                                              const Commands::MoveToHueAndSaturation::DecodableType & commandData)
{
    return ColorControlServer::Instance().moveToHueAndSaturationCommand(
        commandObj, commandPath, commandData.hue, commandData.saturation, commandData.transitionTime, commandData.optionsMask,
        commandData.optionsOverride, false);
}

bool emberAfColorControlClusterStepHueCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                               const Commands::StepHue::DecodableType & commandData)
{
    return ColorControlServer::Instance().stepHueCommand(commandObj, commandPath, commandData.stepMode, commandData.stepSize,
                                                         commandData.transitionTime, commandData.optionsMask,
                                                         commandData.optionsOverride, false);
}

bool emberAfColorControlClusterStepSaturationCallback(app::CommandHandler * commandObj,
                                                      const app::ConcreteCommandPath & commandPath,
                                                      const Commands::StepSaturation::DecodableType & commandData)
{
    return ColorControlServer::Instance().stepSaturationCommand(commandObj, commandPath, commandData);
}

bool emberAfColorControlClusterEnhancedMoveHueCallback(app::CommandHandler * commandObj,
                                                       const app::ConcreteCommandPath & commandPath,
                                                       const Commands::EnhancedMoveHue::DecodableType & commandData)
{
    return ColorControlServer::Instance().moveHueCommand(commandObj, commandPath, commandData.moveMode, commandData.rate,
                                                         commandData.optionsMask, commandData.optionsOverride, true);
}

bool emberAfColorControlClusterEnhancedMoveToHueCallback(app::CommandHandler * commandObj,
                                                         const app::ConcreteCommandPath & commandPath,
                                                         const Commands::EnhancedMoveToHue::DecodableType & commandData)
{
    return ColorControlServer::Instance().moveToHueCommand(commandObj, commandPath, commandData.enhancedHue, commandData.direction,
                                                           commandData.transitionTime, commandData.optionsMask,
                                                           commandData.optionsOverride, true);
}

bool emberAfColorControlClusterEnhancedMoveToHueAndSaturationCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::EnhancedMoveToHueAndSaturation::DecodableType & commandData)
{
    return ColorControlServer::Instance().moveToHueAndSaturationCommand(commandObj, commandPath, commandData.enhancedHue,
                                                                        commandData.saturation, commandData.transitionTime,
                                                                        commandData.optionsMask, commandData.optionsOverride, true);
}

bool emberAfColorControlClusterEnhancedStepHueCallback(app::CommandHandler * commandObj,
                                                       const app::ConcreteCommandPath & commandPath,
                                                       const Commands::EnhancedStepHue::DecodableType & commandData)
{
    return ColorControlServer::Instance().stepHueCommand(commandObj, commandPath, commandData.stepMode, commandData.stepSize,
                                                         commandData.transitionTime, commandData.optionsMask,
                                                         commandData.optionsOverride, true);
}

bool emberAfColorControlClusterColorLoopSetCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                    const Commands::ColorLoopSet::DecodableType & commandData)
{
    return ColorControlServer::Instance().colorLoopCommand(commandObj, commandPath, commandData);
}

#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_XY

bool emberAfColorControlClusterMoveToColorCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                   const Commands::MoveToColor::DecodableType & commandData)
{
    return ColorControlServer::Instance().moveToColorCommand(commandObj, commandPath, commandData);
}

bool emberAfColorControlClusterMoveColorCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                 const Commands::MoveColor::DecodableType & commandData)
{
    return ColorControlServer::Instance().moveColorCommand(commandObj, commandPath, commandData);
}

bool emberAfColorControlClusterStepColorCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                 const Commands::StepColor::DecodableType & commandData)
{
    return ColorControlServer::Instance().stepColorCommand(commandObj, commandPath, commandData);
}

#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_XY

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP

bool emberAfColorControlClusterMoveToColorTemperatureCallback(app::CommandHandler * commandObj,
                                                              const app::ConcreteCommandPath & commandPath,
                                                              const Commands::MoveToColorTemperature::DecodableType & commandData)
{
    return ColorControlServer::Instance().moveToColorTempCommand(commandObj, commandPath, commandData);
}

bool emberAfColorControlClusterMoveColorTemperatureCallback(app::CommandHandler * commandObj,
                                                            const app::ConcreteCommandPath & commandPath,
                                                            const Commands::MoveColorTemperature::DecodableType & commandData)
{
    return ColorControlServer::Instance().moveColorTempCommand(commandObj, commandPath, commandData);
}

bool emberAfColorControlClusterStepColorTemperatureCallback(app::CommandHandler * commandObj,
                                                            const app::ConcreteCommandPath & commandPath,
                                                            const Commands::StepColorTemperature::DecodableType & commandData)
{
    return ColorControlServer::Instance().stepColorTempCommand(commandObj, commandPath, commandData);
}

void emberAfPluginLevelControlCoupledColorTempChangeCallback(EndpointId endpoint)
{
    ColorControlServer::Instance().levelControlColorTempChangeCommand(endpoint);
}

#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP

bool emberAfColorControlClusterStopMoveStepCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                    const Commands::StopMoveStep::DecodableType & commandData)
{
    return ColorControlServer::Instance().stopMoveStepCommand(commandObj, commandPath, commandData.optionsMask,
                                                              commandData.optionsOverride);
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
