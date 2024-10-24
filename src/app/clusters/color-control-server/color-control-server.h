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
#include <app/cluster-building-blocks/QuieterReporting.h>
#include <app/data-model/Nullable.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>
#include <app/util/basic-types.h>
#include <app/util/config.h>
#include <platform/CHIPDeviceConfig.h>
#include <protocols/interaction_model/StatusCode.h>

#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
#include <app/clusters/scenes-server/SceneTable.h>
#endif

/**********************************************************
 * Defines and Macros
 *********************************************************/

static constexpr chip::System::Clock::Milliseconds32 TRANSITION_UPDATE_TIME_MS = chip::System::Clock::Milliseconds32(100);
static constexpr uint16_t TRANSITION_STEPS_PER_1S                              = 10;

static constexpr uint16_t MIN_CIE_XY_VALUE = 0;
static constexpr uint16_t MAX_CIE_XY_VALUE = 0xfeff; // this value comes directly from the ZCL specification table 5.3

// Logically relevant color temperatures are between 1000K and 9000K at the very most (and this is still
// not frequent). Our implementation can default to those reasonable maxima to avoid issues related to range.
static constexpr uint16_t MIN_TEMPERATURE_VALUE = 111u;  // 111 mireds == 9000K
static constexpr uint16_t MAX_TEMPERATURE_VALUE = 1000u; // 1000 mireds == 1000K

static constexpr uint8_t MIN_HUE_VALUE = 0;
static constexpr uint8_t MAX_HUE_VALUE = 254;

static constexpr uint8_t MIN_SATURATION_VALUE = 0;
static constexpr uint8_t MAX_SATURATION_VALUE = 254;

static constexpr uint8_t HALF_MAX_UINT8T   = 127;
static constexpr uint16_t HALF_MAX_UINT16T = 0x7FFF;

static constexpr uint16_t MAX_ENHANCED_HUE_VALUE = 0xFFFF;

static constexpr uint8_t MIN_CURRENT_LEVEL = 0x01;
static constexpr uint8_t MAX_CURRENT_LEVEL = 0xFE;

static constexpr uint8_t REPORT_FAILED = 0xFF;

/**
 * @brief color-control-server class
 */
class ColorControlServer
{
public:
    /**********************************************************
     * Enums
     *********************************************************/
    using StepModeEnum  = chip::app::Clusters::ColorControl::StepModeEnum;
    using MoveModeEnum  = chip::app::Clusters::ColorControl::MoveModeEnum;
    using DirectionEnum = chip::app::Clusters::ColorControl::DirectionEnum;
    using Feature       = chip::app::Clusters::ColorControl::Feature;

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
        // The amount of time remaining until the transition completes. Measured in tenths of a second.
        // When the transition repeats indefinitely, this will hold the maximum value possible.
        uint16_t timeRemaining;
        // The total transitionTime in 1/10th of a seconds
        uint16_t transitionTime;
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
        // The amount of time remaining until the transition completes. Measured in tenths of a second.
        uint16_t timeRemaining;
        // The total transitionTime in 1/10th of a seconds
        uint16_t transitionTime;
        uint16_t lowLimit;
        uint16_t highLimit;
        chip::EndpointId endpoint;
    };

    /**********************************************************
     * Functions Definitions
     *********************************************************/
    static ColorControlServer & Instance();

#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
    chip::scenes::SceneHandler * GetSceneHandler();
#endif

    bool HasFeature(chip::EndpointId endpoint, Feature feature);
    chip::Protocols::InteractionModel::Status stopAllColorTransitions(chip::EndpointId endpoint);
    bool stopMoveStepCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                             chip::BitMask<chip::app::Clusters::ColorControl::OptionsBitmap> optionsMask,
                             chip::BitMask<chip::app::Clusters::ColorControl::OptionsBitmap> optionsOverride);

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV
    bool moveHueCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                        MoveModeEnum moveMode, uint16_t rate,
                        chip::BitMask<chip::app::Clusters::ColorControl::OptionsBitmap> optionsMask,
                        chip::BitMask<chip::app::Clusters::ColorControl::OptionsBitmap> optionsOverride, bool isEnhanced);
    bool moveToHueCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath, uint16_t hue,
                          DirectionEnum moveDirection, uint16_t transitionTime,
                          chip::BitMask<chip::app::Clusters::ColorControl::OptionsBitmap> optionsMask,
                          chip::BitMask<chip::app::Clusters::ColorControl::OptionsBitmap> optionsOverride, bool isEnhanced);
    bool moveToHueAndSaturationCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                                       uint16_t hue, uint8_t saturation, uint16_t transitionTime,
                                       chip::BitMask<chip::app::Clusters::ColorControl::OptionsBitmap> optionsMask,
                                       chip::BitMask<chip::app::Clusters::ColorControl::OptionsBitmap> optionsOverride,
                                       bool isEnhanced);
    bool stepHueCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                        StepModeEnum stepMode, uint16_t stepSize, uint16_t transitionTime,
                        chip::BitMask<chip::app::Clusters::ColorControl::OptionsBitmap> optionsMask,
                        chip::BitMask<chip::app::Clusters::ColorControl::OptionsBitmap> optionsOverride, bool isEnhanced);
    bool moveSaturationCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                               const chip::app::Clusters::ColorControl::Commands::MoveSaturation::DecodableType & commandData);
    bool moveToSaturationCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                                 const chip::app::Clusters::ColorControl::Commands::MoveToSaturation::DecodableType & commandData);
    bool stepSaturationCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                               const chip::app::Clusters::ColorControl::Commands::StepSaturation::DecodableType & commandData);
    bool colorLoopCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                          const chip::app::Clusters::ColorControl::Commands::ColorLoopSet::DecodableType & commandData);
    void updateHueSatCommand(chip::EndpointId endpoint);
#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_XY
    bool moveToColorCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                            const chip::app::Clusters::ColorControl::Commands::MoveToColor::DecodableType & commandData);
    bool moveColorCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                          const chip::app::Clusters::ColorControl::Commands::MoveColor::DecodableType & commandData);
    bool stepColorCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                          const chip::app::Clusters::ColorControl::Commands::StepColor::DecodableType & commandData);
    void updateXYCommand(chip::EndpointId endpoint);
#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_XY

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP
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
#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP

    void cancelEndpointTimerCallback(chip::EndpointId endpoint);

    template <typename Q, typename V>
    chip::app::MarkAttributeDirty SetQuietReportAttribute(chip::app::QuieterReportingAttribute<Q> & quietReporter, V newValue,
                                                          bool isEndOfTransition, uint16_t transitionTime);
    chip::Protocols::InteractionModel::Status SetQuietReportRemainingTime(chip::EndpointId endpoint, uint16_t newRemainingTime,
                                                                          bool isNewTransition = false);

private:
    /**********************************************************
     * Functions Definitions
     *********************************************************/

    ColorControlServer() {}

    bool shouldExecuteIfOff(chip::EndpointId endpoint, chip::BitMask<chip::app::Clusters::ColorControl::OptionsBitmap> optionMask,
                            chip::BitMask<chip::app::Clusters::ColorControl::OptionsBitmap> optionOverride);
    void handleModeSwitch(chip::EndpointId endpoint, chip::app::Clusters::ColorControl::EnhancedColorModeEnum newColorMode);
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
    uint16_t getEndpointIndex(chip::EndpointId);

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV
    chip::Protocols::InteractionModel::Status moveToSaturation(uint8_t saturation, uint16_t transitionTime,
                                                               chip::EndpointId endpoint);
    chip::Protocols::InteractionModel::Status moveToHueAndSaturation(uint16_t hue, uint8_t saturation, uint16_t transitionTime,
                                                                     bool isEnhanced, chip::EndpointId endpoint);
    ColorHueTransitionState * getColorHueTransitionState(chip::EndpointId endpoint);
    Color16uTransitionState * getSaturationTransitionState(chip::EndpointId endpoint);
    ColorHueTransitionState * getColorHueTransitionStateByIndex(uint16_t index);
    Color16uTransitionState * getSaturationTransitionStateByIndex(uint16_t index);
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
#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_XY
    chip::Protocols::InteractionModel::Status moveToColor(uint16_t colorX, uint16_t colorY, uint16_t transitionTime,
                                                          chip::EndpointId endpoint);
    Color16uTransitionState * getXTransitionState(chip::EndpointId endpoint);
    Color16uTransitionState * getYTransitionState(chip::EndpointId endpoint);
    Color16uTransitionState * getXTransitionStateByIndex(uint16_t index);
    Color16uTransitionState * getYTransitionStateByIndex(uint16_t index);
    uint16_t findNewColorValueFromStep(uint16_t oldValue, int16_t step);
    EmberEventControl * configureXYEventControl(chip::EndpointId);
#endif // #ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_XY

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP
    Color16uTransitionState * getTempTransitionState(chip::EndpointId endpoint);
    Color16uTransitionState * getTempTransitionStateByIndex(uint16_t index);
    chip::Protocols::InteractionModel::Status moveToColorTemp(chip::EndpointId aEndpoint, uint16_t colorTemperature,
                                                              uint16_t transitionTime);
    uint16_t getTemperatureCoupleToLevelMin(chip::EndpointId endpoint);
    EmberEventControl * configureTempEventControl(chip::EndpointId);
#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP

    /**********************************************************
     * Attributes Declaration
     *********************************************************/
    static ColorControlServer instance;
    static constexpr size_t kColorControlClusterServerMaxEndpointCount =
        MATTER_DM_COLOR_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;
    static_assert(kColorControlClusterServerMaxEndpointCount <= kEmberInvalidEndpointIndex, "ColorControl endpoint count error");

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV
    ColorHueTransitionState colorHueTransitionStates[kColorControlClusterServerMaxEndpointCount];
    Color16uTransitionState colorSatTransitionStates[kColorControlClusterServerMaxEndpointCount];

    chip::app::QuieterReportingAttribute<uint8_t> quietHue[kColorControlClusterServerMaxEndpointCount];
    chip::app::QuieterReportingAttribute<uint8_t> quietSaturation[kColorControlClusterServerMaxEndpointCount];
    chip::app::QuieterReportingAttribute<uint16_t> quietEnhancedHue[kColorControlClusterServerMaxEndpointCount];
#endif

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_XY
    Color16uTransitionState colorXtransitionStates[kColorControlClusterServerMaxEndpointCount];
    Color16uTransitionState colorYtransitionStates[kColorControlClusterServerMaxEndpointCount];

    chip::app::QuieterReportingAttribute<uint16_t> quietColorX[kColorControlClusterServerMaxEndpointCount];
    chip::app::QuieterReportingAttribute<uint16_t> quietColorY[kColorControlClusterServerMaxEndpointCount];
#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_XY

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP
    Color16uTransitionState colorTempTransitionStates[kColorControlClusterServerMaxEndpointCount];
    chip::app::QuieterReportingAttribute<uint16_t> quietTemperatureMireds[kColorControlClusterServerMaxEndpointCount];
#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP

    EmberEventControl eventControls[kColorControlClusterServerMaxEndpointCount];
    chip::app::QuieterReportingAttribute<uint16_t> quietRemainingTime[kColorControlClusterServerMaxEndpointCount];

#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
    friend class DefaultColorControlSceneHandler;
#endif
};

/**********************************************************
 * Callbacks
 *********************************************************/

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP
void emberAfPluginColorControlServerTempTransitionEventHandler(chip::EndpointId endpoint);
#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_XY
void emberAfPluginColorControlServerXyTransitionEventHandler(chip::EndpointId endpoint);
#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_XY

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV
void emberAfPluginColorControlServerHueSatTransitionEventHandler(chip::EndpointId endpoint);
#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP
void emberAfPluginLevelControlCoupledColorTempChangeCallback(chip::EndpointId endpoint);
#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP
