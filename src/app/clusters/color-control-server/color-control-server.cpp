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

/**
 *
 *    Copyright (c) 2020-21 Silicon Labs
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance  the License.
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
#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af-event.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ColorControl;

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

bool ColorControlServer::HasFeature(chip::EndpointId endpoint, ColorControlFeature feature)
{
    bool success;
    uint32_t featureMap;
    success = (Attributes::FeatureMap::Get(endpoint, &featureMap) == EMBER_ZCL_STATUS_SUCCESS);

    return success ? ((featureMap & to_underlying(feature)) != 0) : false;
}

EmberAfStatus ColorControlServer::stopAllColorTransitions(EndpointId endpoint)
{
    EmberEventControl * event = getEventControl(endpoint);
    VerifyOrReturnError(event != nullptr, EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT);

    emberEventControlSetInactive(event);
    return EMBER_ZCL_STATUS_SUCCESS;
}

bool ColorControlServer::stopMoveStepCommand(EndpointId endpoint, uint8_t optionsMask, uint8_t optionsOverride)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    if (shouldExecuteIfOff(endpoint, optionsMask, optionsOverride))
    {
        status = stopAllColorTransitions(endpoint);
    }

    emberAfSendImmediateDefaultResponse(status);
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
        return READBITS(options, EMBER_ZCL_COLOR_CONTROL_OPTIONS_EXECUTE_IF_OFF);
    }
    // ---------- The above is to distinguish if the payload is present or not

    if (READBITS(optionMask, EMBER_ZCL_COLOR_CONTROL_OPTIONS_EXECUTE_IF_OFF))
    {
        // Mask is present and set in the command payload, this indicates
        // use the override as temporary option
        return READBITS(optionOverride, EMBER_ZCL_COLOR_CONTROL_OPTIONS_EXECUTE_IF_OFF);
    }
    // if we are here - use the option attribute bits
    return (READBITS(options, EMBER_ZCL_COLOR_CONTROL_OPTIONS_EXECUTE_IF_OFF));
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
    if (newColorMode == ColorControlServer::ColorMode::COLOR_MODE_EHSV)
    {
        // Transpose COLOR_MODE_EHSV to COLOR_MODE_HSV after setting EnhancedColorMode
        newColorMode = ColorControlServer::ColorMode::COLOR_MODE_HSV;
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
    uint16_t index            = emberAfFindClusterServerEndpointIndex(endpoint, ColorControl::Id);
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
 * @param p ColorHueTransitionState*
 * @return true command mouvement is finished
 * @return false command mouvement is not finished
 */
bool ColorControlServer::computeNewColor16uValue(ColorControlServer::Color16uTransitionState * p)
{
    uint32_t newValue32u;

    if (p->stepsRemaining == 0)
    {
        return false;
    }

    (p->stepsRemaining)--;

    Attributes::RemainingTime::Set(p->endpoint, p->stepsRemaining);

    // handle sign
    if (p->finalValue == p->currentValue)
    {
        // do nothing
    }
    else if (p->finalValue > p->initialValue)
    {
        newValue32u = ((uint32_t)(p->finalValue - p->initialValue));
        newValue32u *= ((uint32_t)(p->stepsRemaining));
        newValue32u /= ((uint32_t)(p->stepsTotal));
        p->currentValue = static_cast<uint16_t>(p->finalValue - static_cast<uint16_t>(newValue32u));

        if (static_cast<uint16_t>(newValue32u) > p->finalValue || p->currentValue > p->highLimit)
        {
            p->currentValue = p->highLimit;
        }
    }
    else
    {
        newValue32u = ((uint32_t)(p->initialValue - p->finalValue));
        newValue32u *= ((uint32_t)(p->stepsRemaining));
        newValue32u /= ((uint32_t)(p->stepsTotal));
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

#ifdef EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_HSV

/**
 * @brief Returns ColorHueTransititionState associated to an endpoint
 *
 * @param[in] endpoint
 * @return ColorControlServer::ColorHueTransitionState*
 */
ColorControlServer::ColorHueTransitionState * ColorControlServer::getColorHueTransitionState(EndpointId endpoint)
{
    uint16_t index                  = emberAfFindClusterServerEndpointIndex(endpoint, ColorControl::Id);
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
    uint16_t index                  = emberAfFindClusterServerEndpointIndex(endpoint, ColorControl::Id);
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
    ColorHueTransitionState * colorHueTransitionState        = getColorHueTransitionState(endpoint);
    Color16uTransitionState * colorSaturationTransitionState = getSaturationTransitionState(endpoint);

    uint8_t direction = 0;
    Attributes::ColorLoopDirection::Get(endpoint, &direction);

    uint16_t time = 0x0019;
    Attributes::ColorLoopTime::Get(endpoint, &time);

    uint16_t currentHue = 0;
    Attributes::EnhancedCurrentHue::Get(endpoint, &currentHue);

    u_int16_t startHue = 0x2300;
    if (startFromStartHue)
    {
        Attributes::ColorLoopStartEnhancedHue::Get(endpoint, &startHue);
    }
    else
    {
        startHue = currentHue;
    }

    Attributes::ColorLoopStoredEnhancedHue::Set(endpoint, currentHue);
    Attributes::ColorLoopActive::Set(endpoint, true);

    initHueSat(endpoint, colorHueTransitionState, colorSaturationTransitionState);

    colorHueTransitionState->isEnhancedHue = true;

    colorHueTransitionState->initialEnhancedHue = startHue;
    colorHueTransitionState->currentEnhancedHue = currentHue;

    if (direction)
    {
        colorHueTransitionState->finalEnhancedHue = static_cast<uint16_t>(startHue - 1);
    }
    else
    {
        colorHueTransitionState->finalEnhancedHue = static_cast<uint16_t>(startHue + 1);
    }

    colorHueTransitionState->up     = direction;
    colorHueTransitionState->repeat = true;

    colorHueTransitionState->stepsRemaining = static_cast<uint16_t>(time * TRANSITION_TIME_1S);
    colorHueTransitionState->stepsTotal     = static_cast<uint16_t>(time * TRANSITION_TIME_1S);
    colorHueTransitionState->endpoint       = endpoint;

    Attributes::RemainingTime::Set(endpoint, MAX_INT16U_VALUE);

    emberEventControlSetDelayMS(configureHSVEventControl(endpoint), UPDATE_TIME_MS);
}

/**
 * @brief Initialise memory structures for new command
 *
 * @param[in] endpoint
 * @param[out] colorHueTransitionState
 * @param[out] colorSatTransitionState
 */
void ColorControlServer::initHueSat(EndpointId endpoint, ColorControlServer::ColorHueTransitionState * colorHueTransitionState,
                                    ColorControlServer::Color16uTransitionState * colorSatTransitionState)
{
    colorHueTransitionState->stepsRemaining = 0;
    Attributes::CurrentHue::Get(endpoint, &(colorHueTransitionState->currentHue));
    colorHueTransitionState->endpoint = endpoint;

    Attributes::EnhancedCurrentHue::Get(endpoint, &(colorHueTransitionState->currentEnhancedHue));
    colorHueTransitionState->isEnhancedHue = false;

    colorSatTransitionState->stepsRemaining = 0;
    colorSatTransitionState->currentValue   = getSaturation(endpoint);
    colorSatTransitionState->endpoint       = endpoint;
}

/**
 * @brief Computes new hue value based on current position
 *
 * @param p ColorHueTransitionState*
 * @return true command mouvement is finished
 * @return false command mouvement is not finished
 */
bool ColorControlServer::computeNewHueValue(ColorControlServer::ColorHueTransitionState * p)
{
    uint32_t newHue32;
    uint16_t newHue;

    // exit with a false if hue is not currently moving
    if (p->stepsRemaining == 0)
    {
        return false;
    }

    (p->stepsRemaining)--;

    if (p->repeat == false)
    {
        Attributes::RemainingTime::Set(p->endpoint, p->stepsRemaining);
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
        newHue32 /= static_cast<uint32_t>(p->stepsTotal);

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
        newHue32 /= static_cast<uint32_t>(p->stepsTotal);

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
 * @brief Configures EnventControl callback when using HSV colors
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
 * @brief Executes move Hue Command
 *
 * @param[in] endpoint
 * @param[in] moveMode
 * @param[in] rate
 * @param[in] optionsMask
 * @param[in] optionsOverride
 * @param[in] isEnhanced If True, function was called by EnhancedMoveHue command and rate is a uint16 value. If False function was
 * called by MoveHue command and rate is a uint8 value
 * @return true Success
 * @return false Failed
 */
bool ColorControlServer::moveHueCommand(EndpointId endpoint, uint8_t moveMode, uint16_t rate, uint8_t optionsMask,
                                        uint8_t optionsOverride, bool isEnhanced)
{
    uint8_t currentHue          = 0;
    uint16_t currentEnhancedHue = 0;
    EmberAfStatus status        = EMBER_ZCL_STATUS_SUCCESS;

    ColorHueTransitionState * colorHueTransitionState        = getColorHueTransitionState(endpoint);
    Color16uTransitionState * colorSaturationTransitionState = getSaturationTransitionState(endpoint);

    VerifyOrExit(colorHueTransitionState != nullptr, status = EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT);
    VerifyOrExit(colorSaturationTransitionState != nullptr, status = EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT);

    if (!shouldExecuteIfOff(endpoint, optionsMask, optionsOverride))
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
        return true;
    }

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    if (moveMode == EMBER_ZCL_HUE_MOVE_MODE_STOP)
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
        return true;
    }

    // Handle color mode transition, if necessary.
    if (isEnhanced)
    {
        handleModeSwitch(endpoint, ColorControlServer::ColorMode::COLOR_MODE_EHSV);
    }
    else
    {
        handleModeSwitch(endpoint, ColorControlServer::ColorMode::COLOR_MODE_HSV);
    }

    // now, kick off the state machine.
    initHueSat(endpoint, colorHueTransitionState, colorSaturationTransitionState);

    colorHueTransitionState->isEnhancedHue = isEnhanced;
    if (isEnhanced)
    {
        Attributes::EnhancedCurrentHue::Get(endpoint, &currentEnhancedHue);
        colorHueTransitionState->initialEnhancedHue = currentEnhancedHue;
        colorHueTransitionState->currentEnhancedHue = currentEnhancedHue;
    }
    else
    {
        Attributes::CurrentHue::Get(endpoint, &currentHue);
        colorHueTransitionState->initialHue = currentHue;
        colorHueTransitionState->currentHue = currentHue;
    }

    if (moveMode == EMBER_ZCL_HUE_MOVE_MODE_UP)
    {
        if (isEnhanced)
        {
            colorHueTransitionState->finalEnhancedHue = addEnhancedHue(currentEnhancedHue, rate);
        }
        else
        {
            colorHueTransitionState->finalHue = addHue(currentHue, static_cast<uint8_t>(rate));
        }

        colorHueTransitionState->up = true;
    }
    else if (moveMode == EMBER_ZCL_HUE_MOVE_MODE_DOWN)
    {
        if (isEnhanced)
        {
            colorHueTransitionState->finalEnhancedHue = subtractEnhancedHue(currentEnhancedHue, rate);
        }
        else
        {
            colorHueTransitionState->finalHue = subtractHue(currentHue, static_cast<uint8_t>(rate));
        }

        colorHueTransitionState->up = false;
    }
    else
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_MALFORMED_COMMAND);
        return true;
    }
    colorHueTransitionState->stepsRemaining = TRANSITION_TIME_1S;
    colorHueTransitionState->stepsTotal     = TRANSITION_TIME_1S;
    colorHueTransitionState->endpoint       = endpoint;
    colorHueTransitionState->repeat         = true;

    // hue movement can last forever. Indicate this with a remaining time of maxint
    Attributes::RemainingTime::Set(endpoint, MAX_INT16U_VALUE);
    colorSaturationTransitionState->stepsRemaining = 0;

    // kick off the state machine:
    emberEventControlSetDelayMS(configureHSVEventControl(endpoint), UPDATE_TIME_MS);

exit:
    emberAfSendImmediateDefaultResponse(status);
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
 * @param[in] isEnhanced If True, function was called by EnhancedMoveHue command and rate is a uint16 value. If False function was
 * called by MoveHue command and rate is a uint8 value
 * @return true Success
 * @return false Failed
 */
bool ColorControlServer::moveToHueCommand(EndpointId endpoint, uint16_t hue, uint8_t hueMoveMode, uint16_t transitionTime,
                                          uint8_t optionsMask, uint8_t optionsOverride, bool isEnhanced)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    uint16_t currentHue  = 0;
    uint8_t direction;

    ColorHueTransitionState * colorHueTransitionState        = getColorHueTransitionState(endpoint);
    Color16uTransitionState * colorSaturationTransitionState = getSaturationTransitionState(endpoint);

    VerifyOrExit(colorHueTransitionState != nullptr, status = EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT);
    VerifyOrExit(colorSaturationTransitionState != nullptr, status = EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT);

    if (!shouldExecuteIfOff(endpoint, optionsMask, optionsOverride))
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
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

    if (transitionTime == 0)
    {
        transitionTime++;
    }

    // Standard Hue limit checking:  hue is 0..254.  Spec dictates we ignore
    // this and report a malformed packet.
    if (!isEnhanced && (hue > MAX_HUE_VALUE))
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_MALFORMED_COMMAND);
        return true;
    }

    // For move to hue, the move modes are different from the other move commands.
    // Need to translate from the move to hue transitions to the internal
    // representation.
    switch (hueMoveMode)
    {
    case EMBER_ZCL_HUE_DIRECTION_SHORTEST_DISTANCE:
        if ((isEnhanced && (static_cast<uint16_t>(currentHue - hue) > HALF_MAX_UINT16T)) ||
            (!isEnhanced && (static_cast<uint8_t>(currentHue - hue) > HALF_MAX_UINT8T)))
        {
            direction = MOVE_MODE_UP;
        }
        else
        {
            direction = MOVE_MODE_DOWN;
        }
        break;
    case EMBER_ZCL_HUE_DIRECTION_LONGEST_DISTANCE:
        if ((isEnhanced && (static_cast<uint16_t>(currentHue - hue) > HALF_MAX_UINT16T)) ||
            (!isEnhanced && (static_cast<uint8_t>(currentHue - hue) > HALF_MAX_UINT8T)))
        {
            direction = MOVE_MODE_DOWN;
        }
        else
        {
            direction = MOVE_MODE_UP;
        }
        break;
    case EMBER_ZCL_HUE_DIRECTION_UP:
        direction = MOVE_MODE_UP;
        break;
    case EMBER_ZCL_HUE_DIRECTION_DOWN:
        direction = MOVE_MODE_DOWN;
        break;
    default:
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_MALFORMED_COMMAND);
        return true;
    }

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    // Handle color mode transition, if necessary.
    if (isEnhanced)
    {
        handleModeSwitch(endpoint, ColorControlServer::ColorMode::COLOR_MODE_EHSV);
    }
    else
    {
        handleModeSwitch(endpoint, ColorControlServer::ColorMode::COLOR_MODE_HSV);
    }

    // now, kick off the state machine.
    initHueSat(endpoint, colorHueTransitionState, colorSaturationTransitionState);
    colorHueTransitionState->isEnhancedHue = isEnhanced;

    if (isEnhanced)
    {
        Attributes::EnhancedCurrentHue::Get(endpoint, &(colorHueTransitionState->initialEnhancedHue));
        Attributes::EnhancedCurrentHue::Get(endpoint, &(colorHueTransitionState->currentEnhancedHue));

        colorHueTransitionState->finalEnhancedHue = hue;
    }
    else
    {
        Attributes::CurrentHue::Get(endpoint, &(colorHueTransitionState->initialHue));
        Attributes::CurrentHue::Get(endpoint, &(colorHueTransitionState->currentHue));

        colorHueTransitionState->finalHue = static_cast<uint8_t>(hue);
    }

    colorHueTransitionState->stepsRemaining        = transitionTime;
    colorHueTransitionState->stepsTotal            = transitionTime;
    colorHueTransitionState->endpoint              = endpoint;
    colorHueTransitionState->up                    = (direction == MOVE_MODE_UP);
    colorHueTransitionState->repeat                = false;
    colorSaturationTransitionState->stepsRemaining = 0;

    Attributes::RemainingTime::Set(endpoint, transitionTime);

    // kick off the state machine:
    emberEventControlSetDelayMS(configureHSVEventControl(endpoint), UPDATE_TIME_MS);

exit:
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

/**
 * @brief executes move to hue and saturatioan command
 *
 * @param[in] endpoint
 * @param[in] hue
 * @param[in] saturation
 * @param[in] transitionTime
 * @param[in] optionsMask
 * @param[in] optionsOverride
 * @param[in] isEnhanced If True, function was called by EnhancedMoveHue command and rate is a uint16 value. If False function was
 * called by MoveHue command and rate is a uint8 value
 * @return true Success
 * @return false Failed
 */
bool ColorControlServer::moveToHueAndSaturationCommand(EndpointId endpoint, uint16_t hue, uint8_t saturation,
                                                       uint16_t transitionTime, uint8_t optionsMask, uint8_t optionsOverride,
                                                       bool isEnhanced)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    uint16_t currentHue  = 0;
    uint16_t halfWay     = isEnhanced ? HALF_MAX_UINT16T : HALF_MAX_UINT8T;
    bool moveUp;

    Color16uTransitionState * colorSaturationTransitionState = getSaturationTransitionState(endpoint);
    ColorHueTransitionState * colorHueTransitionState        = getColorHueTransitionState(endpoint);

    VerifyOrExit(colorSaturationTransitionState != nullptr, status = EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT);
    VerifyOrExit(colorHueTransitionState != nullptr, status = EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT);

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

    if (transitionTime == 0)
    {
        transitionTime++;
    }

    // limit checking:  hue and saturation are 0..254.  Spec dictates we ignore
    // this and report a malformed packet.
    if ((!isEnhanced && hue > MAX_HUE_VALUE) || saturation > MAX_SATURATION_VALUE)
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_MALFORMED_COMMAND);
        return true;
    }

    if (!shouldExecuteIfOff(endpoint, optionsMask, optionsOverride))
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
        return true;
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

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    // Handle color mode transition, if necessary.
    if (isEnhanced)
    {
        handleModeSwitch(endpoint, ColorControlServer::ColorMode::COLOR_MODE_EHSV);
    }
    else
    {
        handleModeSwitch(endpoint, ColorControlServer::ColorMode::COLOR_MODE_HSV);
    }

    // now, kick off the state machine.
    initHueSat(endpoint, colorHueTransitionState, colorSaturationTransitionState);
    colorHueTransitionState->isEnhancedHue = isEnhanced;

    if (isEnhanced)
    {
        colorHueTransitionState->initialEnhancedHue = currentHue;
        colorHueTransitionState->currentEnhancedHue = currentHue;
        colorHueTransitionState->finalEnhancedHue   = hue;
    }
    else
    {
        colorHueTransitionState->initialHue = static_cast<uint8_t>(currentHue);
        colorHueTransitionState->currentHue = static_cast<uint8_t>(currentHue);
        colorHueTransitionState->finalHue   = static_cast<uint8_t>(hue);
    }

    colorHueTransitionState->stepsRemaining = transitionTime;
    colorHueTransitionState->stepsTotal     = transitionTime;
    colorHueTransitionState->endpoint       = endpoint;
    colorHueTransitionState->up             = moveUp;
    colorHueTransitionState->repeat         = false;

    colorSaturationTransitionState->initialValue   = getSaturation(endpoint);
    colorSaturationTransitionState->currentValue   = getSaturation(endpoint);
    colorSaturationTransitionState->finalValue     = saturation;
    colorSaturationTransitionState->stepsRemaining = transitionTime;
    colorSaturationTransitionState->stepsTotal     = transitionTime;
    colorSaturationTransitionState->endpoint       = endpoint;
    colorSaturationTransitionState->lowLimit       = MIN_SATURATION_VALUE;
    colorSaturationTransitionState->highLimit      = MAX_SATURATION_VALUE;

    Attributes::RemainingTime::Set(endpoint, transitionTime);

    // kick off the state machine:
    emberEventControlSetDelayMS(configureHSVEventControl(endpoint), UPDATE_TIME_MS);

exit:
    emberAfSendImmediateDefaultResponse(status);
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
 * @param[in] isEnhanced If True, function was called by EnhancedMoveHue command and rate is a uint16 value. If False function was
 * called by MoveHue command and rate is a uint8 value
 * @return true Success
 * @return false Failed
 */
bool ColorControlServer::stepHueCommand(EndpointId endpoint, uint8_t stepMode, uint16_t stepSize, uint16_t transitionTime,
                                        uint8_t optionsMask, uint8_t optionsOverride, bool isEnhanced)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    ColorHueTransitionState * colorHueTransitionState        = getColorHueTransitionState(endpoint);
    Color16uTransitionState * colorSaturationTransitionState = getSaturationTransitionState(endpoint);

    VerifyOrExit(colorHueTransitionState != nullptr, status = EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT);
    VerifyOrExit(colorSaturationTransitionState != nullptr, status = EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT);

    if (!shouldExecuteIfOff(endpoint, optionsMask, optionsOverride))
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
        return true;
    }

    if (transitionTime == 0)
    {
        transitionTime++;
    }

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    if (stepMode == MOVE_MODE_STOP)
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
        return true;
    }

    // Handle color mode transition, if necessary.
    if (isEnhanced)
    {
        handleModeSwitch(endpoint, ColorControlServer::ColorMode::COLOR_MODE_EHSV);
    }
    else
    {
        handleModeSwitch(endpoint, ColorControlServer::ColorMode::COLOR_MODE_HSV);
    }

    // now, kick off the state machine.
    initHueSat(endpoint, colorHueTransitionState, colorSaturationTransitionState);
    colorHueTransitionState->isEnhancedHue = isEnhanced;

    if (isEnhanced)
    {
        Attributes::EnhancedCurrentHue::Get(endpoint, &(colorHueTransitionState->currentEnhancedHue));
        colorHueTransitionState->initialEnhancedHue = colorHueTransitionState->currentEnhancedHue;

        if (stepMode == MOVE_MODE_UP)
        {
            colorHueTransitionState->finalEnhancedHue = addEnhancedHue(colorHueTransitionState->currentEnhancedHue, stepSize);
            colorHueTransitionState->up               = true;
        }
        else
        {
            colorHueTransitionState->finalEnhancedHue = subtractEnhancedHue(colorHueTransitionState->currentEnhancedHue, stepSize);
            colorHueTransitionState->up               = false;
        }
    }
    else
    {
        Attributes::CurrentHue::Get(endpoint, &(colorHueTransitionState->currentHue));
        colorHueTransitionState->initialHue = colorHueTransitionState->currentHue;

        if (stepMode == MOVE_MODE_UP)
        {
            colorHueTransitionState->finalHue = addHue(colorHueTransitionState->currentHue, static_cast<uint8_t>(stepSize));
            colorHueTransitionState->up       = true;
        }
        else
        {
            colorHueTransitionState->finalHue = subtractHue(colorHueTransitionState->currentHue, static_cast<uint8_t>(stepSize));
            colorHueTransitionState->up       = false;
        }
    }

    colorHueTransitionState->stepsRemaining        = transitionTime;
    colorHueTransitionState->stepsTotal            = transitionTime;
    colorHueTransitionState->endpoint              = endpoint;
    colorHueTransitionState->repeat                = false;
    colorSaturationTransitionState->stepsRemaining = 0;

    Attributes::RemainingTime::Set(endpoint, transitionTime);

    // kick off the state machine:
    emberEventControlSetDelayMS(configureHSVEventControl(endpoint), UPDATE_TIME_MS);

exit:
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool ColorControlServer::moveSaturationCommand(const app::ConcreteCommandPath & commandPath,
                                               const Commands::MoveSaturation::DecodableType & commandData)
{
    auto & moveMode        = commandData.moveMode;
    auto & rate            = commandData.rate;
    auto & optionsMask     = commandData.optionsMask;
    auto & optionsOverride = commandData.optionsOverride;
    EndpointId endpoint    = commandPath.mEndpointId;
    EmberAfStatus status   = EMBER_ZCL_STATUS_SUCCESS;

    ColorHueTransitionState * colorHueTransitionState        = getColorHueTransitionState(endpoint);
    Color16uTransitionState * colorSaturationTransitionState = getSaturationTransitionState(endpoint);

    VerifyOrExit(colorHueTransitionState != nullptr, status = EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT);
    VerifyOrExit(colorSaturationTransitionState != nullptr, status = EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT);

    if (!shouldExecuteIfOff(endpoint, optionsMask, optionsOverride))
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
        return true;
    }

    uint16_t transitionTime;

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    if (moveMode == EMBER_ZCL_SATURATION_MOVE_MODE_STOP || rate == 0)
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
        return true;
    }

    // Handle color mode transition, if necessary.
    handleModeSwitch(endpoint, COLOR_MODE_HSV);

    // now, kick off the state machine.
    initHueSat(endpoint, colorHueTransitionState, colorSaturationTransitionState);

    colorHueTransitionState->stepsRemaining = 0;

    colorSaturationTransitionState->initialValue = getSaturation(endpoint);
    colorSaturationTransitionState->currentValue = getSaturation(endpoint);
    if (moveMode == EMBER_ZCL_SATURATION_MOVE_MODE_UP)
    {
        colorSaturationTransitionState->finalValue = MAX_SATURATION_VALUE;
    }
    else
    {
        colorSaturationTransitionState->finalValue = MIN_SATURATION_VALUE;
    }

    transitionTime = computeTransitionTimeFromStateAndRate(colorSaturationTransitionState, rate);

    colorSaturationTransitionState->stepsRemaining = transitionTime;
    colorSaturationTransitionState->stepsTotal     = transitionTime;
    colorSaturationTransitionState->endpoint       = endpoint;
    colorSaturationTransitionState->lowLimit       = MIN_SATURATION_VALUE;
    colorSaturationTransitionState->highLimit      = MAX_SATURATION_VALUE;

    Attributes::RemainingTime::Set(endpoint, transitionTime);

    // kick off the state machine:
    emberEventControlSetDelayMS(configureHSVEventControl(endpoint), UPDATE_TIME_MS);

exit:
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

/**
 * @brief executes move to saturation command
 *
 * @param saturation
 * @param transitionTime
 * @param optionsMask
 * @param optionsOverride
 * @return true
 * @return false
 */
bool ColorControlServer::moveToSaturationCommand(const app::ConcreteCommandPath & commandPath,
                                                 const Commands::MoveToSaturation::DecodableType & commandData)
{
    uint8_t saturation      = commandData.saturation;
    uint16_t transitionTime = commandData.transitionTime;
    uint8_t optionsMask     = commandData.optionsMask;
    uint8_t optionsOverride = commandData.optionsOverride;
    EndpointId endpoint     = commandPath.mEndpointId;
    EmberAfStatus status    = EMBER_ZCL_STATUS_SUCCESS;

    ColorHueTransitionState * colorHueTransitionState        = getColorHueTransitionState(endpoint);
    Color16uTransitionState * colorSaturationTransitionState = getSaturationTransitionState(endpoint);

    VerifyOrExit(colorHueTransitionState != nullptr, status = EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT);
    VerifyOrExit(colorSaturationTransitionState != nullptr, status = EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT);

    if (!shouldExecuteIfOff(endpoint, optionsMask, optionsOverride))
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
        return true;
    }

    if (transitionTime == 0)
    {
        transitionTime++;
    }

    // limit checking:  hue and saturation are 0..254.  Spec dictates we ignore
    // this and report a malformed packet.
    if (saturation > MAX_SATURATION_VALUE)
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_MALFORMED_COMMAND);
        return true;
    }

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    // Handle color mode transition, if necessary.
    handleModeSwitch(endpoint, COLOR_MODE_HSV);

    // now, kick off the state machine.
    initHueSat(endpoint, colorHueTransitionState, colorSaturationTransitionState);

    colorHueTransitionState->stepsRemaining = 0;

    colorSaturationTransitionState->initialValue   = getSaturation(endpoint);
    colorSaturationTransitionState->currentValue   = getSaturation(endpoint);
    colorSaturationTransitionState->finalValue     = saturation;
    colorSaturationTransitionState->stepsRemaining = transitionTime;
    colorSaturationTransitionState->stepsTotal     = transitionTime;
    colorSaturationTransitionState->endpoint       = endpoint;
    colorSaturationTransitionState->lowLimit       = MIN_SATURATION_VALUE;
    colorSaturationTransitionState->highLimit      = MAX_SATURATION_VALUE;

    Attributes::RemainingTime::Set(endpoint, transitionTime);

    // kick off the state machine:
    emberEventControlSetDelayMS(configureHSVEventControl(endpoint), UPDATE_TIME_MS);

exit:
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool ColorControlServer::stepSaturationCommand(const app::ConcreteCommandPath & commandPath,
                                               const Commands::StepSaturation::DecodableType & commandData)
{
    uint8_t stepMode          = commandData.stepMode;
    uint8_t stepSize          = commandData.stepSize;
    uint8_t transitionTime    = commandData.transitionTime;
    uint8_t optionsMask       = commandData.optionsMask;
    uint8_t optionsOverride   = commandData.optionsOverride;
    EndpointId endpoint       = commandPath.mEndpointId;
    EmberAfStatus status      = EMBER_ZCL_STATUS_SUCCESS;
    uint8_t currentSaturation = 0;

    ColorHueTransitionState * colorHueTransitionState        = getColorHueTransitionState(endpoint);
    Color16uTransitionState * colorSaturationTransitionState = getSaturationTransitionState(endpoint);

    VerifyOrExit(colorHueTransitionState != nullptr, status = EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT);
    VerifyOrExit(colorSaturationTransitionState != nullptr, status = EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT);

    if (!shouldExecuteIfOff(endpoint, optionsMask, optionsOverride))
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
        return true;
    }

    currentSaturation = getSaturation(endpoint);

    if (transitionTime == 0)
    {
        transitionTime++;
    }

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    if (stepMode == MOVE_MODE_STOP)
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
        return true;
    }

    // Handle color mode transition, if necessary.
    handleModeSwitch(endpoint, COLOR_MODE_HSV);

    // now, kick off the state machine.
    initHueSat(endpoint, colorHueTransitionState, colorSaturationTransitionState);

    colorHueTransitionState->stepsRemaining = 0;

    colorSaturationTransitionState->initialValue = currentSaturation;
    colorSaturationTransitionState->currentValue = currentSaturation;

    if (stepMode == MOVE_MODE_UP)
    {
        colorSaturationTransitionState->finalValue = addSaturation(currentSaturation, stepSize);
    }
    else
    {
        colorSaturationTransitionState->finalValue = subtractSaturation(currentSaturation, stepSize);
    }
    colorSaturationTransitionState->stepsRemaining = transitionTime;
    colorSaturationTransitionState->stepsTotal     = transitionTime;
    colorSaturationTransitionState->endpoint       = endpoint;
    colorSaturationTransitionState->lowLimit       = MIN_SATURATION_VALUE;
    colorSaturationTransitionState->highLimit      = MAX_SATURATION_VALUE;

    Attributes::RemainingTime::Set(endpoint, transitionTime);

    // kick off the state machine:
    emberEventControlSetDelayMS(configureHSVEventControl(endpoint), UPDATE_TIME_MS);

exit:
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool ColorControlServer::colorLoopCommand(const app::ConcreteCommandPath & commandPath,
                                          const Commands::ColorLoopSet::DecodableType & commandData)
{
    uint8_t updateFlags       = commandData.updateFlags.Raw();
    uint8_t action            = commandData.action;
    uint8_t direction         = commandData.direction;
    uint16_t time             = commandData.time;
    uint16_t startHue         = commandData.startHue;
    uint8_t optionsMask       = commandData.optionsMask;
    uint8_t optionsOverride   = commandData.optionsOverride;
    EndpointId endpoint       = commandPath.mEndpointId;
    EmberAfStatus status      = EMBER_ZCL_STATUS_SUCCESS;
    uint8_t isColorLoopActive = 0;
    uint8_t deactiveColorLoop = 0;

    ColorHueTransitionState * colorHueTransitionState = getColorHueTransitionState(endpoint);
    VerifyOrExit(colorHueTransitionState != nullptr, status = EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT);

    if (!shouldExecuteIfOff(endpoint, optionsMask, optionsOverride))
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
        return true;
    }

    Attributes::ColorLoopActive::Get(endpoint, &isColorLoopActive);

    deactiveColorLoop =
        (updateFlags & EMBER_AF_COLOR_LOOP_UPDATE_FLAGS_UPDATE_ACTION) && (action == EMBER_ZCL_COLOR_LOOP_ACTION_DEACTIVATE);

    if (updateFlags & EMBER_AF_COLOR_LOOP_UPDATE_FLAGS_UPDATE_DIRECTION)
    {
        Attributes::ColorLoopDirection::Set(endpoint, direction);

        // Checks if color loop is active and stays active
        if (isColorLoopActive && !deactiveColorLoop)
        {
            colorHueTransitionState->up                 = direction;
            colorHueTransitionState->initialEnhancedHue = colorHueTransitionState->currentEnhancedHue;

            if (direction)
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

    if (updateFlags & EMBER_AF_COLOR_LOOP_UPDATE_FLAGS_UPDATE_TIME)
    {
        Attributes::ColorLoopTime::Set(endpoint, time);

        // Checks if color loop is active and stays active
        if (isColorLoopActive && !deactiveColorLoop)
        {
            colorHueTransitionState->stepsTotal         = static_cast<uint16_t>(time * TRANSITION_TIME_1S);
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

    if (updateFlags & EMBER_AF_COLOR_LOOP_UPDATE_FLAGS_UPDATE_START_HUE)
    {
        Attributes::ColorLoopStartEnhancedHue::Set(endpoint, startHue);
    }

    if (updateFlags & EMBER_AF_COLOR_LOOP_UPDATE_FLAGS_UPDATE_ACTION)
    {
        if (action == EMBER_ZCL_COLOR_LOOP_ACTION_DEACTIVATE)
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
        else if (action == EMBER_ZCL_COLOR_LOOP_ACTION_ACTIVATE_FROM_COLOR_LOOP_START_ENHANCED_HUE)
        {
            startColorLoop(endpoint, true);
        }
        else if (action == EMBER_ZCL_COLOR_LOOP_ACTION_ACTIVATE_FROM_ENHANCED_CURRENT_HUE)
        {
            startColorLoop(endpoint, false);
        }
        else
        {
            emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_MALFORMED_COMMAND);
            return true;
        }
    }

exit:
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

/**
 * @brief updates Hue and saturation after timer is finished
 *
 * @param endpoint
 */
void ColorControlServer::updateHueSatCommand(EndpointId endpoint)
{
    bool limitReached1, limitReached2;

    ColorHueTransitionState * colorHueTransitionState        = getColorHueTransitionState(endpoint);
    Color16uTransitionState * colorSaturationTransitionState = getSaturationTransitionState(endpoint);

    limitReached1 = computeNewHueValue(colorHueTransitionState);
    limitReached2 = computeNewColor16uValue(colorSaturationTransitionState);

    if (limitReached1 || limitReached2)
    {
        stopAllColorTransitions(endpoint);
    }
    else
    {
        emberEventControlSetDelayMS(configureHSVEventControl(endpoint), UPDATE_TIME_MS);
    }

    if (colorHueTransitionState->isEnhancedHue)
    {
        Attributes::EnhancedCurrentHue::Set(endpoint, colorHueTransitionState->currentEnhancedHue);
        Attributes::CurrentHue::Set(endpoint, static_cast<uint8_t>(colorHueTransitionState->currentEnhancedHue >> 8));
    }
    else
    {
        Attributes::CurrentHue::Set(colorHueTransitionState->endpoint, colorHueTransitionState->currentHue);
    }

    Attributes::CurrentSaturation::Set(colorSaturationTransitionState->endpoint,
                                       (uint8_t) colorSaturationTransitionState->currentValue);
    if (colorHueTransitionState->isEnhancedHue)
    {
        emberAfColorControlClusterPrintln("Enhanced Hue %d Saturation %d endpoint %d", colorHueTransitionState->currentEnhancedHue,
                                          colorSaturationTransitionState->currentValue, endpoint);
    }
    else
    {
        emberAfColorControlClusterPrintln("Hue %d Saturation %d endpoint %d", colorHueTransitionState->currentHue,
                                          colorSaturationTransitionState->currentValue, endpoint);
    }
    computePwmFromHsv(endpoint);
}

#endif // EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_HSV

#ifdef EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_XY

/**
 * @brief Returns Color16uTransitionState for X color associated to an endpoint
 *
 * @param endpoint
 * @return ColorControlServer::Color16uTransitionState*
 */
ColorControlServer::Color16uTransitionState * ColorControlServer::getXTransitionState(EndpointId endpoint)
{
    uint16_t index = emberAfFindClusterServerEndpointIndex(endpoint, ColorControl::Id);

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
    uint16_t index = emberAfFindClusterServerEndpointIndex(endpoint, ColorControl::Id);

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
 * @brief Configures EnventControl callback when using XY colors
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

bool ColorControlServer::moveToColorCommand(const app::ConcreteCommandPath & commandPath,
                                            const Commands::MoveToColor::DecodableType & commandData)
{
    uint16_t colorX         = commandData.colorX;
    uint16_t colorY         = commandData.colorY;
    uint16_t transitionTime = commandData.transitionTime;
    uint8_t optionsMask     = commandData.optionsMask;
    uint8_t optionsOverride = commandData.optionsOverride;
    EndpointId endpoint     = commandPath.mEndpointId;
    EmberAfStatus status    = EMBER_ZCL_STATUS_SUCCESS;

    Color16uTransitionState * colorXTransitionState = getXTransitionState(endpoint);
    Color16uTransitionState * colorYTransitionState = getYTransitionState(endpoint);

    VerifyOrExit(colorXTransitionState != nullptr, status = EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT);
    VerifyOrExit(colorYTransitionState != nullptr, status = EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT);

    if (!shouldExecuteIfOff(endpoint, optionsMask, optionsOverride))
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
        return true;
    }

    if (transitionTime == 0)
    {
        transitionTime++;
    }

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    // Handle color mode transition, if necessary.
    handleModeSwitch(endpoint, COLOR_MODE_CIE_XY);

    // now, kick off the state machine.
    Attributes::CurrentX::Get(endpoint, &(colorXTransitionState->initialValue));
    Attributes::CurrentX::Get(endpoint, &(colorXTransitionState->currentValue));
    colorXTransitionState->finalValue     = colorX;
    colorXTransitionState->stepsRemaining = transitionTime;
    colorXTransitionState->stepsTotal     = transitionTime;
    colorXTransitionState->endpoint       = endpoint;
    colorXTransitionState->lowLimit       = MIN_CIE_XY_VALUE;
    colorXTransitionState->highLimit      = MAX_CIE_XY_VALUE;

    Attributes::CurrentY::Get(endpoint, &(colorYTransitionState->initialValue));
    Attributes::CurrentY::Get(endpoint, &(colorYTransitionState->currentValue));
    colorYTransitionState->finalValue     = colorY;
    colorYTransitionState->stepsRemaining = transitionTime;
    colorYTransitionState->stepsTotal     = transitionTime;
    colorYTransitionState->endpoint       = endpoint;
    colorYTransitionState->lowLimit       = MIN_CIE_XY_VALUE;
    colorYTransitionState->highLimit      = MAX_CIE_XY_VALUE;

    Attributes::RemainingTime::Set(endpoint, transitionTime);

    // kick off the state machine:
    emberEventControlSetDelayMS(configureXYEventControl(endpoint), UPDATE_TIME_MS);

exit:
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool ColorControlServer::moveColorCommand(const app::ConcreteCommandPath & commandPath,
                                          const Commands::MoveColor::DecodableType & commandData)
{
    int16_t rateX           = commandData.rateX;
    int16_t rateY           = commandData.rateY;
    uint8_t optionsMask     = commandData.optionsMask;
    uint8_t optionsOverride = commandData.optionsOverride;
    EndpointId endpoint     = commandPath.mEndpointId;
    EmberAfStatus status    = EMBER_ZCL_STATUS_SUCCESS;

    Color16uTransitionState * colorXTransitionState = getXTransitionState(endpoint);
    Color16uTransitionState * colorYTransitionState = getYTransitionState(endpoint);

    VerifyOrExit(colorXTransitionState != nullptr, status = EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT);
    VerifyOrExit(colorYTransitionState != nullptr, status = EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT);

    if (!shouldExecuteIfOff(endpoint, optionsMask, optionsOverride))
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
        return true;
    }

    uint16_t transitionTimeX, transitionTimeY;
    uint16_t unsignedRate;

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    if (rateX == 0 && rateY == 0)
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
        return true;
    }

    // Handle color mode transition, if necessary.
    handleModeSwitch(endpoint, COLOR_MODE_CIE_XY);

    // now, kick off the state machine.
    Attributes::CurrentX::Get(endpoint, &(colorXTransitionState->initialValue));
    colorXTransitionState->currentValue = colorXTransitionState->initialValue;
    if (rateX > 0)
    {
        colorXTransitionState->finalValue = MAX_CIE_XY_VALUE;
        unsignedRate                      = (uint16_t) rateX;
    }
    else
    {
        colorXTransitionState->finalValue = MIN_CIE_XY_VALUE;
        unsignedRate                      = (uint16_t)(rateX * -1);
    }
    transitionTimeX                       = computeTransitionTimeFromStateAndRate(colorXTransitionState, unsignedRate);
    colorXTransitionState->stepsRemaining = transitionTimeX;
    colorXTransitionState->stepsTotal     = transitionTimeX;
    colorXTransitionState->endpoint       = endpoint;
    colorXTransitionState->lowLimit       = MIN_CIE_XY_VALUE;
    colorXTransitionState->highLimit      = MAX_CIE_XY_VALUE;

    Attributes::CurrentY::Get(endpoint, &(colorYTransitionState->initialValue));
    colorYTransitionState->currentValue = colorYTransitionState->initialValue;
    if (rateY > 0)
    {
        colorYTransitionState->finalValue = MAX_CIE_XY_VALUE;
        unsignedRate                      = (uint16_t) rateY;
    }
    else
    {
        colorYTransitionState->finalValue = MIN_CIE_XY_VALUE;
        unsignedRate                      = (uint16_t)(rateY * -1);
    }
    transitionTimeY                       = computeTransitionTimeFromStateAndRate(colorYTransitionState, unsignedRate);
    colorYTransitionState->stepsRemaining = transitionTimeY;
    colorYTransitionState->stepsTotal     = transitionTimeY;
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
    emberEventControlSetDelayMS(configureXYEventControl(endpoint), UPDATE_TIME_MS);

exit:
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool ColorControlServer::stepColorCommand(const app::ConcreteCommandPath & commandPath,
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

    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    Color16uTransitionState * colorXTransitionState = getXTransitionState(endpoint);
    Color16uTransitionState * colorYTransitionState = getYTransitionState(endpoint);

    VerifyOrExit(colorXTransitionState != nullptr, status = EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT);
    VerifyOrExit(colorYTransitionState != nullptr, status = EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT);

    if (!shouldExecuteIfOff(endpoint, optionsMask, optionsOverride))
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
        return true;
    }

    Attributes::CurrentX::Get(endpoint, &currentColorX);
    Attributes::CurrentY::Get(endpoint, &currentColorY);

    colorX = findNewColorValueFromStep(currentColorX, stepX);
    colorY = findNewColorValueFromStep(currentColorY, stepY);

    if (transitionTime == 0)
    {
        transitionTime++;
    }

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    // Handle color mode transition, if necessary.
    handleModeSwitch(endpoint, COLOR_MODE_CIE_XY);

    // now, kick off the state machine.
    colorXTransitionState->initialValue   = currentColorX;
    colorXTransitionState->currentValue   = currentColorX;
    colorXTransitionState->finalValue     = colorX;
    colorXTransitionState->stepsRemaining = transitionTime;
    colorXTransitionState->stepsTotal     = transitionTime;
    colorXTransitionState->endpoint       = endpoint;
    colorXTransitionState->lowLimit       = MIN_CIE_XY_VALUE;
    colorXTransitionState->highLimit      = MAX_CIE_XY_VALUE;

    colorYTransitionState->initialValue   = currentColorY;
    colorYTransitionState->currentValue   = currentColorY;
    colorYTransitionState->finalValue     = colorY;
    colorYTransitionState->stepsRemaining = transitionTime;
    colorYTransitionState->stepsTotal     = transitionTime;
    colorYTransitionState->endpoint       = endpoint;
    colorYTransitionState->lowLimit       = MIN_CIE_XY_VALUE;
    colorYTransitionState->highLimit      = MAX_CIE_XY_VALUE;

    Attributes::RemainingTime::Set(endpoint, transitionTime);

    // kick off the state machine:
    emberEventControlSetDelayMS(configureXYEventControl(endpoint), UPDATE_TIME_MS);

exit:
    emberAfSendImmediateDefaultResponse(status);
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
    bool limitReachedX, limitReachedY;

    // compute new values for X and Y.
    limitReachedX = computeNewColor16uValue(colorXTransitionState);

    limitReachedY = computeNewColor16uValue(colorYTransitionState);

    if (limitReachedX || limitReachedY)
    {
        stopAllColorTransitions(endpoint);
    }
    else
    {
        emberEventControlSetDelayMS(configureXYEventControl(endpoint), UPDATE_TIME_MS);
    }

    // update the attributes
    Attributes::CurrentX::Set(endpoint, colorXTransitionState->currentValue);
    Attributes::CurrentY::Set(endpoint, colorYTransitionState->currentValue);

    emberAfColorControlClusterPrintln("Color X %d Color Y %d", colorXTransitionState->currentValue,
                                      colorYTransitionState->currentValue);

    computePwmFromXy(endpoint);
}

#endif // EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_XY

#ifdef EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_TEMP
/**
 * @brief Get the Temp Transition State object associated to the endpoint
 *
 * @param endpoint
 * @return Color16uTransitionState*
 */
ColorControlServer::Color16uTransitionState * ColorControlServer::getTempTransitionState(EndpointId endpoint)
{
    uint16_t index = emberAfFindClusterServerEndpointIndex(endpoint, ColorControl::Id);

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
 */
EmberAfStatus ColorControlServer::moveToColorTemp(EndpointId aEndpoint, uint16_t colorTemperature, uint16_t transitionTime)
{
    EndpointId endpoint = aEndpoint;

    Color16uTransitionState * colorTempTransitionState = getTempTransitionState(endpoint);
    VerifyOrReturnError(colorTempTransitionState != nullptr, EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT);

    uint16_t temperatureMin = MIN_TEMPERATURE_VALUE;
    Attributes::ColorTempPhysicalMinMireds::Get(endpoint, &temperatureMin);

    uint16_t temperatureMax = MAX_TEMPERATURE_VALUE;
    Attributes::ColorTempPhysicalMaxMireds::Get(endpoint, &temperatureMax);

    if (transitionTime == 0)
    {
        transitionTime++;
    }

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    // Handle color mode transition, if necessary.
    handleModeSwitch(endpoint, COLOR_MODE_TEMPERATURE);

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
    colorTempTransitionState->stepsRemaining = transitionTime;
    colorTempTransitionState->stepsTotal     = transitionTime;
    colorTempTransitionState->endpoint       = endpoint;
    colorTempTransitionState->lowLimit       = temperatureMin;
    colorTempTransitionState->highLimit      = temperatureMax;

    // kick off the state machine
    emberEventControlSetDelayMS(configureTempEventControl(endpoint), UPDATE_TIME_MS);
    return EMBER_ZCL_STATUS_SUCCESS;
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
    EmberAfStatus status;

    status = Attributes::CoupleColorTempToLevelMinMireds::Get(endpoint, &colorTemperatureCoupleToLevelMin);

    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        // Not less than the physical min.
        Attributes::ColorTempPhysicalMinMireds::Get(endpoint, &colorTemperatureCoupleToLevelMin);
    }

    return colorTemperatureCoupleToLevelMin;
}

/**
 * @brief Configures EnventControl callback when using Temp colors
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
    // temperature values a lamp SHAL use when it is supplied with power and this value SHALL
    // be reflected in the ColorTemperatureMireds attribute. In addition, the ColorMode and
    // EnhancedColorMode attributes SHALL be set to 0x02 (color temperature). The values of
    // the StartUpColorTemperatureMireds attribute are listed in the table below.
    // Value                Action on power up
    // 0x0000-0xffef        Set the ColorTemperatureMireds attribute to this value.
    // null                 Set the ColorTemperatureMireds attribute to its previous value.

    // Initialize startUpColorTempMireds to "maintain previous value" value null
    app::DataModel::Nullable<uint16_t> startUpColorTemp;
    EmberAfStatus status = Attributes::StartUpColorTemperatureMireds::Get(endpoint, startUpColorTemp);

    if (status == EMBER_ZCL_STATUS_SUCCESS && !startUpColorTemp.IsNull())
    {
        uint16_t updatedColorTemp = MAX_TEMPERATURE_VALUE;
        status                    = Attributes::ColorTemperatureMireds::Get(endpoint, &updatedColorTemp);

        if (status == EMBER_ZCL_STATUS_SUCCESS)
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

                if (status == EMBER_ZCL_STATUS_SUCCESS)
                {
                    // Set ColorMode attributes to reflect ColorTemperature.
                    uint8_t updateColorMode = EMBER_ZCL_COLOR_MODE_COLOR_TEMPERATURE;
                    Attributes::ColorMode::Set(endpoint, updateColorMode);

                    updateColorMode = EMBER_ZCL_ENHANCED_COLOR_MODE_COLOR_TEMPERATURE;
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
    bool limitReached;

    limitReached = computeNewColor16uValue(colorTempTransitionState);

    if (limitReached)
    {
        stopAllColorTransitions(endpoint);
    }
    else
    {
        emberEventControlSetDelayMS(configureTempEventControl(endpoint), UPDATE_TIME_MS);
    }

    Attributes::ColorTemperatureMireds::Set(endpoint, colorTempTransitionState->currentValue);

    emberAfColorControlClusterPrintln("Color Temperature %d", colorTempTransitionState->currentValue);

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
bool ColorControlServer::moveColorTempCommand(const app::ConcreteCommandPath & commandPath,
                                              const Commands::MoveColorTemperature::DecodableType & commandData)
{
    uint8_t moveMode                 = commandData.moveMode;
    uint16_t rate                    = commandData.rate;
    uint16_t colorTemperatureMinimum = commandData.colorTemperatureMinimumMireds;
    uint16_t colorTemperatureMaximum = commandData.colorTemperatureMaximumMireds;
    uint8_t optionsMask              = commandData.optionsMask;
    uint8_t optionsOverride          = commandData.optionsOverride;
    EndpointId endpoint              = commandPath.mEndpointId;
    EmberAfStatus status             = EMBER_ZCL_STATUS_SUCCESS;
    uint16_t tempPhysicalMin         = MIN_TEMPERATURE_VALUE;
    uint16_t tempPhysicalMax         = MAX_TEMPERATURE_VALUE;
    uint16_t transitionTime;

    Color16uTransitionState * colorTempTransitionState = getTempTransitionState(endpoint);
    VerifyOrExit(colorTempTransitionState != nullptr, status = EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT);

    if (!shouldExecuteIfOff(endpoint, optionsMask, optionsOverride))
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
        return true;
    }

    Attributes::ColorTempPhysicalMinMireds::Get(endpoint, &tempPhysicalMin);
    Attributes::ColorTempPhysicalMaxMireds::Get(endpoint, &tempPhysicalMax);

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    if (moveMode == MOVE_MODE_STOP)
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
        return true;
    }

    if (rate == 0)
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_FIELD);
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
    handleModeSwitch(endpoint, COLOR_MODE_TEMPERATURE);

    // now, kick off the state machine.
    colorTempTransitionState->initialValue = 0;
    Attributes::ColorTemperatureMireds::Get(endpoint, &colorTempTransitionState->initialValue);
    colorTempTransitionState->currentValue = colorTempTransitionState->initialValue;

    if (moveMode == MOVE_MODE_UP)
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
    colorTempTransitionState->endpoint       = endpoint;
    colorTempTransitionState->lowLimit       = colorTemperatureMinimum;
    colorTempTransitionState->highLimit      = colorTemperatureMaximum;

    Attributes::RemainingTime::Set(endpoint, transitionTime);

    // kick off the state machine:
    emberEventControlSetDelayMS(configureTempEventControl(endpoint), UPDATE_TIME_MS);

exit:
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool ColorControlServer::moveToColorTempCommand(const app::ConcreteCommandPath & commandPath,
                                                const Commands::MoveToColorTemperature::DecodableType & commandData)
{
    uint16_t colorTemperature = commandData.colorTemperature;
    uint16_t transitionTime   = commandData.transitionTime;
    uint8_t optionsMask       = commandData.optionsMask;
    uint8_t optionsOverride   = commandData.optionsOverride;
    EndpointId endpoint       = commandPath.mEndpointId;

    if (!shouldExecuteIfOff(endpoint, optionsMask, optionsOverride))
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
        return true;
    }

    EmberAfStatus error = moveToColorTemp(endpoint, colorTemperature, transitionTime);

    emberAfSendImmediateDefaultResponse(error);
    return true;
}

bool ColorControlServer::stepColorTempCommand(const app::ConcreteCommandPath & commandPath,
                                              const Commands::StepColorTemperature::DecodableType & commandData)
{
    uint8_t stepMode                 = commandData.stepMode;
    uint16_t stepSize                = commandData.stepSize;
    uint16_t transitionTime          = commandData.transitionTime;
    uint16_t colorTemperatureMinimum = commandData.colorTemperatureMinimumMireds;
    uint16_t colorTemperatureMaximum = commandData.colorTemperatureMaximumMireds;
    uint8_t optionsMask              = commandData.optionsMask;
    uint8_t optionsOverride          = commandData.optionsOverride;
    EndpointId endpoint              = commandPath.mEndpointId;
    EmberAfStatus status             = EMBER_ZCL_STATUS_SUCCESS;
    uint16_t tempPhysicalMin         = MIN_TEMPERATURE_VALUE;
    uint16_t tempPhysicalMax         = MAX_TEMPERATURE_VALUE;

    Color16uTransitionState * colorTempTransitionState = getTempTransitionState(endpoint);
    VerifyOrExit(colorTempTransitionState != nullptr, status = EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT);

    if (!shouldExecuteIfOff(endpoint, optionsMask, optionsOverride))
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
        return true;
    }

    Attributes::ColorTempPhysicalMinMireds::Get(endpoint, &tempPhysicalMin);
    Attributes::ColorTempPhysicalMaxMireds::Get(endpoint, &tempPhysicalMax);

    if (transitionTime == 0)
    {
        transitionTime++;
    }

    // New command.  Need to stop any active transitions.
    stopAllColorTransitions(endpoint);

    if (stepMode == MOVE_MODE_STOP)
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
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
    handleModeSwitch(endpoint, COLOR_MODE_TEMPERATURE);

    // now, kick off the state machine.
    colorTempTransitionState->initialValue = 0;
    Attributes::ColorTemperatureMireds::Get(endpoint, &colorTempTransitionState->initialValue);
    colorTempTransitionState->currentValue = colorTempTransitionState->initialValue;

    if (stepMode == MOVE_MODE_UP)
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
    else
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
    colorTempTransitionState->stepsRemaining = transitionTime;
    colorTempTransitionState->stepsTotal     = transitionTime;
    colorTempTransitionState->endpoint       = endpoint;
    colorTempTransitionState->lowLimit       = colorTemperatureMinimum;
    colorTempTransitionState->highLimit      = colorTemperatureMaximum;

    Attributes::RemainingTime::Set(endpoint, transitionTime);

    // kick off the state machine:
    emberEventControlSetDelayMS(configureTempEventControl(endpoint), UPDATE_TIME_MS);

exit:
    emberAfSendImmediateDefaultResponse(status);
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

    if (colorMode == COLOR_MODE_TEMPERATURE)
    {
        uint16_t tempCoupleMin = getTemperatureCoupleToLevelMin(endpoint);

        app::DataModel::Nullable<uint8_t> currentLevel;
        EmberAfStatus status = LevelControl::Attributes::CurrentLevel::Get(endpoint, currentLevel);

        if (status != EMBER_ZCL_STATUS_SUCCESS || currentLevel.IsNull())
        {
            currentLevel.SetNonNull((uint8_t) 0x7F);
        }

        uint16_t tempPhysMax = MAX_TEMPERATURE_VALUE;
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
            uint32_t tempDelta = (((uint32_t) tempPhysMax - (uint32_t) tempCoupleMin) * currentLevel.Value()) /
                (uint32_t)(MAX_CURRENT_LEVEL - MIN_CURRENT_LEVEL + 1);
            newColorTemp = (uint16_t)((uint32_t) tempPhysMax - tempDelta);
        }

        // Apply new color temp.
        moveToColorTemp(endpoint, newColorTemp, 0);
    }
}

#endif // EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_TEMP

/**********************************************************
 * Callbacks Implementation
 *********************************************************/

#ifdef EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_HSV

bool emberAfColorControlClusterMoveHueCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                               const Commands::MoveHue::DecodableType & commandData)
{
    return ColorControlServer::Instance().moveHueCommand(commandPath.mEndpointId, commandData.moveMode, commandData.rate,
                                                         commandData.optionsMask, commandData.optionsOverride, false);
}

bool emberAfColorControlClusterMoveSaturationCallback(app::CommandHandler * commandObj,
                                                      const app::ConcreteCommandPath & commandPath,
                                                      const Commands::MoveSaturation::DecodableType & commandData)
{

    return ColorControlServer::Instance().moveSaturationCommand(commandPath, commandData);
}

bool emberAfColorControlClusterMoveToHueCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                 const Commands::MoveToHue::DecodableType & commandData)
{
    return ColorControlServer::Instance().moveToHueCommand(commandPath.mEndpointId, commandData.hue, commandData.direction,
                                                           commandData.transitionTime, commandData.optionsMask,
                                                           commandData.optionsOverride, false);
}

bool emberAfColorControlClusterMoveToSaturationCallback(app::CommandHandler * commandObj,
                                                        const app::ConcreteCommandPath & commandPath,
                                                        const Commands::MoveToSaturation::DecodableType & commandData)
{
    return ColorControlServer::Instance().moveToSaturationCommand(commandPath, commandData);
}

bool emberAfColorControlClusterMoveToHueAndSaturationCallback(app::CommandHandler * commandObj,
                                                              const app::ConcreteCommandPath & commandPath,
                                                              const Commands::MoveToHueAndSaturation::DecodableType & commandData)
{
    return ColorControlServer::Instance().moveToHueAndSaturationCommand(
        commandPath.mEndpointId, commandData.hue, commandData.saturation, commandData.transitionTime, commandData.optionsMask,
        commandData.optionsOverride, false);
}

bool emberAfColorControlClusterStepHueCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                               const Commands::StepHue::DecodableType & commandData)
{
    return ColorControlServer::Instance().stepHueCommand(commandPath.mEndpointId, commandData.stepMode, commandData.stepSize,
                                                         commandData.transitionTime, commandData.optionsMask,
                                                         commandData.optionsOverride, false);
}

bool emberAfColorControlClusterStepSaturationCallback(app::CommandHandler * commandObj,
                                                      const app::ConcreteCommandPath & commandPath,
                                                      const Commands::StepSaturation::DecodableType & commandData)
{
    return ColorControlServer::Instance().stepSaturationCommand(commandPath, commandData);
}

bool emberAfColorControlClusterEnhancedMoveHueCallback(app::CommandHandler * commandObj,
                                                       const app::ConcreteCommandPath & commandPath,
                                                       const Commands::EnhancedMoveHue::DecodableType & commandData)
{
    return ColorControlServer::Instance().moveHueCommand(commandPath.mEndpointId, commandData.moveMode, commandData.rate,
                                                         commandData.optionsMask, commandData.optionsOverride, true);
}

bool emberAfColorControlClusterEnhancedMoveToHueCallback(app::CommandHandler * commandObj,
                                                         const app::ConcreteCommandPath & commandPath,
                                                         const Commands::EnhancedMoveToHue::DecodableType & commandData)
{
    return ColorControlServer::Instance().moveToHueCommand(commandPath.mEndpointId, commandData.enhancedHue, commandData.direction,
                                                           commandData.transitionTime, commandData.optionsMask,
                                                           commandData.optionsOverride, true);
}

bool emberAfColorControlClusterEnhancedMoveToHueAndSaturationCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::EnhancedMoveToHueAndSaturation::DecodableType & commandData)
{
    return ColorControlServer::Instance().moveToHueAndSaturationCommand(commandPath.mEndpointId, commandData.enhancedHue,
                                                                        commandData.saturation, commandData.transitionTime,
                                                                        commandData.optionsMask, commandData.optionsOverride, true);
}

bool emberAfColorControlClusterEnhancedStepHueCallback(app::CommandHandler * commandObj,
                                                       const app::ConcreteCommandPath & commandPath,
                                                       const Commands::EnhancedStepHue::DecodableType & commandData)
{
    return ColorControlServer::Instance().stepHueCommand(commandPath.mEndpointId, commandData.stepMode, commandData.stepSize,
                                                         commandData.transitionTime, commandData.optionsMask,
                                                         commandData.optionsOverride, true);
}

bool emberAfColorControlClusterColorLoopSetCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                    const Commands::ColorLoopSet::DecodableType & commandData)
{
    return ColorControlServer::Instance().colorLoopCommand(commandPath, commandData);
}

#endif // EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_HSV

#ifdef EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_XY

bool emberAfColorControlClusterMoveToColorCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                   const Commands::MoveToColor::DecodableType & commandData)
{
    return ColorControlServer::Instance().moveToColorCommand(commandPath, commandData);
}

bool emberAfColorControlClusterMoveColorCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                 const Commands::MoveColor::DecodableType & commandData)
{
    return ColorControlServer::Instance().moveColorCommand(commandPath, commandData);
}

bool emberAfColorControlClusterStepColorCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                 const Commands::StepColor::DecodableType & commandData)
{
    return ColorControlServer::Instance().stepColorCommand(commandPath, commandData);
}

#endif // EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_XY

#ifdef EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_TEMP

bool emberAfColorControlClusterMoveToColorTemperatureCallback(app::CommandHandler * commandObj,
                                                              const app::ConcreteCommandPath & commandPath,
                                                              const Commands::MoveToColorTemperature::DecodableType & commandData)
{
    return ColorControlServer::Instance().moveToColorTempCommand(commandPath, commandData);
}

bool emberAfColorControlClusterMoveColorTemperatureCallback(app::CommandHandler * commandObj,
                                                            const app::ConcreteCommandPath & commandPath,
                                                            const Commands::MoveColorTemperature::DecodableType & commandData)
{
    return ColorControlServer::Instance().moveColorTempCommand(commandPath, commandData);
}

bool emberAfColorControlClusterStepColorTemperatureCallback(app::CommandHandler * commandObj,
                                                            const app::ConcreteCommandPath & commandPath,
                                                            const Commands::StepColorTemperature::DecodableType & commandData)
{
    return ColorControlServer::Instance().stepColorTempCommand(commandPath, commandData);
}

void emberAfPluginLevelControlCoupledColorTempChangeCallback(EndpointId endpoint)
{
    ColorControlServer::Instance().levelControlColorTempChangeCommand(endpoint);
}

#endif // EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_TEMP

bool emberAfColorControlClusterStopMoveStepCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                    const Commands::StopMoveStep::DecodableType & commandData)
{
    return ColorControlServer::Instance().stopMoveStepCommand(commandPath.mEndpointId, commandData.optionsMask,
                                                              commandData.optionsOverride);
}

void emberAfColorControlClusterServerInitCallback(EndpointId endpoint)
{
#ifdef EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_TEMP
    ColorControlServer::Instance().startUpColorTempCommand(endpoint);
#endif // EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_TEMP
}

#ifdef EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_TEMP
/**
 * @brief Callback for temperature update when timer is finished
 *
 * @param endpoint
 */
void emberAfPluginColorControlServerTempTransitionEventHandler(EndpointId endpoint)
{
    ColorControlServer::Instance().updateTempCommand(endpoint);
}
#endif // EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_TEMP

#ifdef EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_XY
/**
 * @brief Callback for color update when timer is finished
 *
 * @param endpoint
 */
void emberAfPluginColorControlServerXyTransitionEventHandler(EndpointId endpoint)
{
    ColorControlServer::Instance().updateXYCommand(endpoint);
}
#endif // EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_XY

#ifdef EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_HSV
/**
 * @brief Callback for color hue and saturation update when timer is finished
 *
 * @param endpoint
 */
void emberAfPluginColorControlServerHueSatTransitionEventHandler(EndpointId endpoint)
{
    ColorControlServer::Instance().updateHueSatCommand(endpoint);
}
#endif // EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_HSV

void MatterColorControlPluginServerInitCallback() {}
