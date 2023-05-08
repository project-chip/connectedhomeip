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

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af-types.h>
#include <app/util/basic-types.h>
#include <app/util/config.h>
#include <platform/CHIPDeviceConfig.h>
#include <protocols/interaction_model/StatusCode.h>

/**********************************************************
 * Defines and Macros
 *********************************************************/

#define UPDATE_TIME_MS 100
#define TRANSITION_TIME_1S 10

#define MIN_CIE_XY_VALUE 0
#define MAX_CIE_XY_VALUE 0xfeff // this value comes directly from the ZCL specification table 5.3

#define MIN_TEMPERATURE_VALUE 0
#define MAX_TEMPERATURE_VALUE 0xfeff

#define MIN_HUE_VALUE 0
#define MAX_HUE_VALUE 254

#define MIN_SATURATION_VALUE 0
#define MAX_SATURATION_VALUE 254

#define HALF_MAX_UINT8T 127
#define HALF_MAX_UINT16T 0x7FFF

#define MAX_ENHANCED_HUE_VALUE 0xFFFF

#define MIN_CURRENT_LEVEL 0x01
#define MAX_CURRENT_LEVEL 0xFE

#define REPORT_FAILED 0xFF

using chip::app::Clusters::ColorControl::ColorControlFeature;

/**
 * @brief color-control-server class
 */
class ColorControlServer
{
public:
    /**********************************************************
     * Enums
     *********************************************************/
    using HueStepMode  = chip::app::Clusters::ColorControl::HueStepMode;
    using HueMoveMode  = chip::app::Clusters::ColorControl::HueMoveMode;
    using HueDirection = chip::app::Clusters::ColorControl::HueDirection;

    enum ColorMode
    {
        COLOR_MODE_HSV         = 0x00,
        COLOR_MODE_CIE_XY      = 0x01,
        COLOR_MODE_TEMPERATURE = 0x02,
        COLOR_MODE_EHSV        = 0x03
    };

    enum Conversion
    {
        HSV_TO_HSV                 = 0x00,
        HSV_TO_CIE_XY              = 0x01,
        HSV_TO_TEMPERATURE         = 0x02,
        CIE_XY_TO_HSV              = 0x10,
        CIE_XY_TO_CIE_XY           = 0x11,
        CIE_XY_TO_TEMPERATURE      = 0x12,
        TEMPERATURE_TO_HSV         = 0x20,
        TEMPERATURE_TO_CIE_XY      = 0x21,
        TEMPERATURE_TO_TEMPERATURE = 0x22
    };

    /**********************************************************
     * Structures
     *********************************************************/

    struct ColorHueTransitionState
    {
        uint8_t initialHue;
        uint8_t currentHue;
        uint8_t finalHue;
        uint16_t stepsRemaining;
        uint16_t stepsTotal;
        uint16_t initialEnhancedHue;
        uint16_t currentEnhancedHue;
        uint16_t finalEnhancedHue;
        chip::EndpointId endpoint;
        bool up;
        bool repeat;
        bool isEnhancedHue;
    };

    struct Color16uTransitionState
    {
        uint16_t initialValue;
        uint16_t currentValue;
        uint16_t finalValue;
        uint16_t stepsRemaining;
        uint16_t stepsTotal;
        uint16_t lowLimit;
        uint16_t highLimit;
        chip::EndpointId endpoint;
    };

    /**********************************************************
     * Functions Definitions
     *********************************************************/
    static ColorControlServer & Instance();

    bool HasFeature(chip::EndpointId endpoint, ColorControlFeature feature);
    chip::Protocols::InteractionModel::Status stopAllColorTransitions(chip::EndpointId endpoint);
    bool stopMoveStepCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                             uint8_t optionsMask, uint8_t optionsOverride);

#ifdef EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_HSV
    bool moveHueCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                        HueMoveMode moveMode, uint16_t rate, uint8_t optionsMask, uint8_t optionsOverride, bool isEnhanced);
    bool moveToHueCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath, uint16_t hue,
                          HueDirection moveDirection, uint16_t transitionTime, uint8_t optionsMask, uint8_t optionsOverride,
                          bool isEnhanced);
    bool moveToHueAndSaturationCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                                       uint16_t hue, uint8_t saturation, uint16_t transitionTime, uint8_t optionsMask,
                                       uint8_t optionsOverride, bool isEnhanced);
    bool stepHueCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                        HueStepMode stepMode, uint16_t stepSize, uint16_t transitionTime, uint8_t optionsMask,
                        uint8_t optionsOverride, bool isEnhanced);
    bool moveSaturationCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                               const chip::app::Clusters::ColorControl::Commands::MoveSaturation::DecodableType & commandData);
    bool moveToSaturationCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                                 const chip::app::Clusters::ColorControl::Commands::MoveToSaturation::DecodableType & commandData);
    bool stepSaturationCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                               const chip::app::Clusters::ColorControl::Commands::StepSaturation::DecodableType & commandData);
    bool colorLoopCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                          const chip::app::Clusters::ColorControl::Commands::ColorLoopSet::DecodableType & commandData);
    void updateHueSatCommand(chip::EndpointId endpoint);
#endif // EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_HSV

#ifdef EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_XY
    bool moveToColorCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                            const chip::app::Clusters::ColorControl::Commands::MoveToColor::DecodableType & commandData);
    bool moveColorCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                          const chip::app::Clusters::ColorControl::Commands::MoveColor::DecodableType & commandData);
    bool stepColorCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                          const chip::app::Clusters::ColorControl::Commands::StepColor::DecodableType & commandData);
    void updateXYCommand(chip::EndpointId endpoint);
#endif // EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_XY

#ifdef EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_TEMP
    bool moveColorTempCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                              const chip::app::Clusters::ColorControl::Commands::MoveColorTemperature::DecodableType & commandData);
    bool
    moveToColorTempCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                           const chip::app::Clusters::ColorControl::Commands::MoveToColorTemperature::DecodableType & commandData);
    bool stepColorTempCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                              const chip::app::Clusters::ColorControl::Commands::StepColorTemperature::DecodableType & commandData);
    void levelControlColorTempChangeCommand(chip::EndpointId endpoint);
    void startUpColorTempCommand(chip::EndpointId endpoint);
    void updateTempCommand(chip::EndpointId endpoint);
#endif // EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_TEMP

    void cancelEndpointTimerCallback(chip::EndpointId endpoint);

private:
    /**********************************************************
     * Functions Definitions
     *********************************************************/

    ColorControlServer() {}
    bool shouldExecuteIfOff(chip::EndpointId endpoint, uint8_t optionMask, uint8_t optionOverride);
    void handleModeSwitch(chip::EndpointId endpoint, uint8_t newColorMode);
    uint16_t computeTransitionTimeFromStateAndRate(Color16uTransitionState * p, uint16_t rate);
    EmberEventControl * getEventControl(chip::EndpointId endpoint);
    void computePwmFromHsv(chip::EndpointId endpoint);
    void computePwmFromTemp(chip::EndpointId endpoint);
    void computePwmFromXy(chip::EndpointId endpoint);
    bool computeNewColor16uValue(Color16uTransitionState * p);

    // Matter timer scheduling glue logic
    static void timerCallback(chip::System::Layer *, void * callbackContext);
    void scheduleTimerCallbackMs(EmberEventControl * control, uint32_t delayMs);
    void cancelEndpointTimerCallback(EmberEventControl * control);

#ifdef EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_HSV
    ColorHueTransitionState * getColorHueTransitionState(chip::EndpointId endpoint);
    Color16uTransitionState * getSaturationTransitionState(chip::EndpointId endpoint);
    uint8_t getSaturation(chip::EndpointId endpoint);
    uint8_t addHue(uint8_t hue1, uint8_t hue2);
    uint8_t subtractHue(uint8_t hue1, uint8_t hue2);
    uint8_t addSaturation(uint8_t saturation1, uint8_t saturation2);
    uint8_t subtractSaturation(uint8_t saturation1, uint8_t saturation2);
    uint16_t addEnhancedHue(uint16_t hue1, uint16_t hue2);
    uint16_t subtractEnhancedHue(uint16_t hue1, uint16_t hue2);
    void startColorLoop(chip::EndpointId endpoint, uint8_t startFromStartHue);
    void initHueTransitionState(chip::EndpointId endpoint, ColorHueTransitionState * colorHueTransitionState, bool isEnhancedHue);
    void initSaturationTransitionState(chip::EndpointId endpoint, Color16uTransitionState * colorSatTransitionState);
    void SetHSVRemainingTime(chip::EndpointId endpoint);
    bool computeNewHueValue(ColorHueTransitionState * p);
    EmberEventControl * configureHSVEventControl(chip::EndpointId);
#endif // EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_HSV

#ifdef EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_XY
    Color16uTransitionState * getXTransitionState(chip::EndpointId endpoint);
    Color16uTransitionState * getYTransitionState(chip::EndpointId endpoint);
    uint16_t findNewColorValueFromStep(uint16_t oldValue, int16_t step);
    EmberEventControl * configureXYEventControl(chip::EndpointId);
#endif // #ifdef EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_XY

#ifdef EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_TEMP
    Color16uTransitionState * getTempTransitionState(chip::EndpointId endpoint);
    chip::Protocols::InteractionModel::Status moveToColorTemp(chip::EndpointId aEndpoint, uint16_t colorTemperature,
                                                              uint16_t transitionTime);
    uint16_t getTemperatureCoupleToLevelMin(chip::EndpointId endpoint);
    EmberEventControl * configureTempEventControl(chip::EndpointId);
#endif // EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_TEMP

    /**********************************************************
     * Attributes Declaration
     *********************************************************/
    static ColorControlServer instance;

#ifdef EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_HSV
    ColorHueTransitionState colorHueTransitionStates[EMBER_AF_COLOR_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT];
    Color16uTransitionState colorSatTransitionStates[EMBER_AF_COLOR_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT];
#endif

#ifdef EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_XY
    Color16uTransitionState colorXtransitionStates[EMBER_AF_COLOR_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT];
    Color16uTransitionState colorYtransitionStates[EMBER_AF_COLOR_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT];
#endif // EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_XY

#ifdef EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_TEMP
    Color16uTransitionState colorTempTransitionStates[EMBER_AF_COLOR_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT];
#endif // EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_TEMP

    EmberEventControl eventControls[EMBER_AF_COLOR_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT];
};

/**********************************************************
 * Callbacks
 *********************************************************/

#ifdef EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_TEMP
void emberAfPluginColorControlServerTempTransitionEventHandler(chip::EndpointId endpoint);
#endif // EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_TEMP

#ifdef EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_XY
void emberAfPluginColorControlServerXyTransitionEventHandler(chip::EndpointId endpoint);
#endif // EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_XY

#ifdef EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_HSV
void emberAfPluginColorControlServerHueSatTransitionEventHandler(chip::EndpointId endpoint);
#endif // EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_HSV

#ifdef EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_TEMP
void emberAfPluginLevelControlCoupledColorTempChangeCallback(chip::EndpointId endpoint);
#endif // EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_TEMP
