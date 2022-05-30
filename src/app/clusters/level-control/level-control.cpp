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
 *    Copyright (c) 2020 Silicon Labs
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
/****************************************************************************
 * @file
 * @brief Routines for the Level Control plugin, which
 *implements the Level Control cluster.
 *******************************************************************************
 ******************************************************************************/

// clusters specific header
#include "level-control.h"

// this file contains all the common includes for clusters in the util
#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af.h>
#include <app/util/util.h>

#include <app/reporting/reporting.h>
#include <platform/CHIPDeviceConfig.h>

#ifdef EMBER_AF_PLUGIN_SCENES
#include <app/clusters/scenes/scenes.h>
#endif // EMBER_AF_PLUGIN_SCENES

#ifdef EMBER_AF_PLUGIN_ON_OFF
#include <app/clusters/on-off-server/on-off-server.h>
#endif // EMBER_AF_PLUGIN_ON_OFF

#ifdef EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_TEMP
#include <app/clusters/color-control-server/color-control-server.h>
#endif // EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_TEMP

#include <assert.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LevelControl;

#ifndef IGNORE_LEVEL_CONTROL_CLUSTER_START_UP_CURRENT_LEVEL
static bool areStartUpLevelControlServerAttributesNonVolatile(EndpointId endpoint);
#endif // IGNORE_LEVEL_CONTROL_CLUSTER_START_UP_CURRENT_LEVEL

#if (EMBER_AF_PLUGIN_LEVEL_CONTROL_RATE == 0)
#define FASTEST_TRANSITION_TIME_MS 0
#else
#define FASTEST_TRANSITION_TIME_MS (MILLISECOND_TICKS_PER_SECOND / EMBER_AF_PLUGIN_LEVEL_CONTROL_RATE)
#endif // EMBER_AF_PLUGIN_LEVEL_CONTROL_RATE

#define LEVEL_CONTROL_LIGHTING_MIN_LEVEL 0x01
#define LEVEL_CONTROL_LIGHTING_MAX_LEVEL 0xFE

#define INVALID_STORED_LEVEL 0xFFFF

#define STARTUP_CURRENT_LEVEL_USE_DEVICE_MINIMUM 0x00
#define STARTUP_CURRENT_LEVEL_USE_PREVIOUS_LEVEL 0xFF

static constexpr size_t kLevelControlStateTableSize =
    EMBER_AF_LEVEL_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

typedef struct
{
    CommandId commandId;
    uint8_t moveToLevel;
    bool increasing;
    bool useOnLevel;
    uint8_t onLevel;
    uint8_t minLevel;
    uint8_t maxLevel;
    uint16_t storedLevel;
    uint32_t eventDurationMs;
    uint32_t transitionTimeMs;
    uint32_t elapsedTimeMs;
} EmberAfLevelControlState;

static EmberAfLevelControlState stateTable[kLevelControlStateTableSize];

static EmberAfLevelControlState * getState(EndpointId endpoint);

static EmberAfStatus moveToLevelHandler(EndpointId endpoint, CommandId commandId, uint8_t level, uint16_t transitionTimeDs,
                                        uint8_t optionMask, uint8_t optionOverride, uint16_t storedLevel);
static void moveHandler(CommandId commandId, uint8_t moveMode, uint8_t rate, uint8_t optionMask, uint8_t optionOverride);
static void stepHandler(CommandId commandId, uint8_t stepMode, uint8_t stepSize, uint16_t transitionTimeDs, uint8_t optionMask,
                        uint8_t optionOverride);
static void stopHandler(CommandId commandId, uint8_t optionMask, uint8_t optionOverride);

static void setOnOffValue(EndpointId endpoint, bool onOff);
static void writeRemainingTime(EndpointId endpoint, uint16_t remainingTimeMs);
static bool shouldExecuteIfOff(EndpointId endpoint, CommandId commandId, uint8_t optionMask, uint8_t optionOverride);

#if !defined(IGNORE_LEVEL_CONTROL_CLUSTER_OPTIONS) && defined(EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_TEMP)
static void reallyUpdateCoupledColorTemp(EndpointId endpoint);
#define updateCoupledColorTemp(endpoint) reallyUpdateCoupledColorTemp(endpoint)
#else
#define updateCoupledColorTemp(endpoint)
#endif // IGNORE_LEVEL_CONTROL_CLUSTER_OPTIONS && EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_TEMP

static void schedule(EndpointId endpoint, uint32_t delayMs)
{
    emberAfScheduleServerTickExtended(endpoint, LevelControl::Id, delayMs, EMBER_AF_LONG_POLL, EMBER_AF_OK_TO_SLEEP);
}

static void deactivate(EndpointId endpoint)
{
    emberAfDeactivateServerTick(endpoint, LevelControl::Id);
}

static EmberAfLevelControlState * getState(EndpointId endpoint)
{
    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, LevelControl::Id);
    return (ep == 0xFFFF ? nullptr : &stateTable[ep]);
}

#if !defined(IGNORE_LEVEL_CONTROL_CLUSTER_OPTIONS) && defined(EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_TEMP)
static void reallyUpdateCoupledColorTemp(EndpointId endpoint)
{
    uint8_t options;
    EmberAfStatus status = Attributes::Options::Get(endpoint, &options);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfLevelControlClusterPrintln("Unable to read Options attribute: 0x%X", status);
        return;
    }

    if (emberAfContainsAttribute(endpoint, ColorControl::Id, ColorControl::Attributes::ColorTemperature::Id))
    {
        if (READBITS(options, EMBER_ZCL_LEVEL_CONTROL_OPTIONS_COUPLE_COLOR_TEMP_TO_LEVEL))
        {
            emberAfPluginLevelControlCoupledColorTempChangeCallback(endpoint);
        }
    }
}
#endif // IGNORE_LEVEL_CONTROL_CLUSTER_OPTIONS && EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_TEMP

void emberAfLevelControlClusterServerTickCallback(EndpointId endpoint)
{
    EmberAfLevelControlState * state = getState(endpoint);
    EmberAfStatus status;
    uint8_t currentLevel;

    if (state == nullptr)
    {
        return;
    }

    state->elapsedTimeMs += state->eventDurationMs;

    // Read the attribute; print error message and return if it can't be read
    status = Attributes::CurrentLevel::Get(endpoint, &currentLevel);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfLevelControlClusterPrintln("ERR: reading current level %x", status);
        writeRemainingTime(endpoint, 0);
        return;
    }

    emberAfLevelControlClusterPrint("Event: move from %d", currentLevel);

    // adjust by the proper amount, either up or down
    if (state->transitionTimeMs == 0)
    {
        // Immediate, not over a time interval.
        currentLevel = state->moveToLevel;
    }
    else if (state->increasing)
    {
        assert(currentLevel < state->maxLevel);
        assert(currentLevel < state->moveToLevel);
        currentLevel++;
    }
    else
    {
        assert(state->minLevel < currentLevel);
        assert(state->moveToLevel < currentLevel);
        currentLevel--;
    }

    emberAfLevelControlClusterPrint(" to %d ", currentLevel);
    emberAfLevelControlClusterPrintln("(diff %c1)", state->increasing ? '+' : '-');

    status = Attributes::CurrentLevel::Set(endpoint, currentLevel);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfLevelControlClusterPrintln("ERR: writing current level %x", status);
        writeRemainingTime(endpoint, 0);
        return;
    }

    updateCoupledColorTemp(endpoint);

#ifdef EMBER_AF_PLUGIN_SCENES
    // The level has changed, so the scene is no longer valid.
    if (emberAfContainsServer(endpoint, Scenes::Id))
    {
        emberAfScenesClusterMakeInvalidCallback(endpoint);
    }
#endif // EMBER_AF_PLUGIN_SCENES

    // Are we at the requested level?
    if (currentLevel == state->moveToLevel)
    {
        if (state->commandId == Commands::MoveToLevelWithOnOff::Id || state->commandId == Commands::MoveWithOnOff::Id ||
            state->commandId == Commands::StepWithOnOff::Id)
        {
            setOnOffValue(endpoint, (currentLevel != state->minLevel));
            if (currentLevel == state->minLevel && state->useOnLevel)
            {
                status = Attributes::CurrentLevel::Set(endpoint, state->onLevel);
                if (status != EMBER_ZCL_STATUS_SUCCESS)
                {
                    emberAfLevelControlClusterPrintln("ERR: writing current level %x", status);
                }
                else
                {
                    updateCoupledColorTemp(endpoint);
                }
            }
        }
        else
        {
            if (state->storedLevel != INVALID_STORED_LEVEL)
            {
                uint8_t storedLevel8u = (uint8_t) state->storedLevel;
                status                = Attributes::CurrentLevel::Set(endpoint, storedLevel8u);
                if (status != EMBER_ZCL_STATUS_SUCCESS)
                {
                    emberAfLevelControlClusterPrintln("ERR: writing current level %x", status);
                }
                else
                {
                    updateCoupledColorTemp(endpoint);
                }
            }
        }
        writeRemainingTime(endpoint, 0);
    }
    else
    {
        writeRemainingTime(endpoint, static_cast<uint16_t>(state->transitionTimeMs - state->elapsedTimeMs));
        schedule(endpoint, state->eventDurationMs);
    }
}

static void writeRemainingTime(EndpointId endpoint, uint16_t remainingTimeMs)
{
#ifndef IGNORE_LEVEL_CONTROL_CLUSTER_LEVEL_CONTROL_REMAINING_TIME
    if (emberAfContainsAttribute(endpoint, LevelControl::Id, LevelControl::Attributes::RemainingTime::Id))
    {
        // Convert milliseconds to tenths of a second, rounding any fractional value
        // up to the nearest whole value.  This means:
        //
        //   0 ms = 0.00 ds = 0 ds
        //   1 ms = 0.01 ds = 1 ds
        //   ...
        //   100 ms = 1.00 ds = 1 ds
        //   101 ms = 1.01 ds = 2 ds
        //   ...
        //   200 ms = 2.00 ds = 2 ds
        //   201 ms = 2.01 ds = 3 ds
        //   ...
        //
        // This is done to ensure that the attribute, in tenths of a second, only
        // goes to zero when the remaining time in milliseconds is actually zero.
        uint16_t remainingTimeDs = static_cast<uint16_t>((remainingTimeMs + 99) / 100);
        EmberStatus status       = LevelControl::Attributes::RemainingTime::Set(endpoint, remainingTimeDs);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            emberAfLevelControlClusterPrintln("ERR: writing remaining time %x", status);
        }
    }
#endif // IGNORE_LEVEL_CONTROL_CLUSTER_LEVEL_CONTROL_REMAINING_TIME
}

static void setOnOffValue(EndpointId endpoint, bool onOff)
{
#ifdef EMBER_AF_PLUGIN_ON_OFF
    if (emberAfContainsServer(endpoint, OnOff::Id))
    {
        emberAfLevelControlClusterPrintln("Setting on/off to %p due to level change", onOff ? "ON" : "OFF");
        OnOffServer::Instance().setOnOffValue(endpoint, (onOff ? OnOff::Commands::On::Id : OnOff::Commands::Off::Id), true);
    }
#endif // EMBER_AF_PLUGIN_ON_OFF
}

static bool shouldExecuteIfOff(EndpointId endpoint, CommandId commandId, uint8_t optionMask, uint8_t optionOverride)
{
#ifndef IGNORE_LEVEL_CONTROL_CLUSTER_OPTIONS
    if (emberAfContainsAttribute(endpoint, LevelControl::Id, Attributes::Options::Id))
    {
        // From 3.10.2.2.8.1 of ZCL7 document 14-0127-20j-zcl-ch-3-general.docx:
        //   "Command execution SHALL NOT continue beyond the Options processing if
        //    all of these criteria are true:
        //      - The command is one of the ‘without On/Off’ commands: Move, Move to
        //        Level, Stop, or Step.
        //      - The On/Off cluster exists on the same endpoint as this cluster.
        //      - The OnOff attribute of the On/Off cluster, on this endpoint, is 0x00
        //        (FALSE).
        //      - The value of the ExecuteIfOff bit is 0."
        if (commandId > Commands::Stop::Id)
        {
            return true;
        }

        if (!emberAfContainsServer(endpoint, OnOff::Id))
        {
            return true;
        }

        uint8_t options;
        EmberAfStatus status = Attributes::Options::Get(endpoint, &options);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            emberAfLevelControlClusterPrintln("Unable to read Options attribute: 0x%X", status);
            // If we can't read the attribute, then we should just assume that it has its
            // default value.
            options = 0x00;
        }

        bool on;
        status = OnOff::Attributes::OnOff::Get(endpoint, &on);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            emberAfLevelControlClusterPrintln("Unable to read OnOff attribute: 0x%X", status);
            return true;
        }
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
        // in section 3.10.2.2.3.

        // ---------- The following order is important in decision making -------
        // -----------more readable ----------
        //
        if (optionMask == 0xFF && optionOverride == 0xFF)
        {
            // 0xFF are the default values passed to the command handler when
            // the payload is not present - in that case there is use of option
            // attribute to decide execution of the command
            return READBITS(options, EMBER_ZCL_LEVEL_CONTROL_OPTIONS_EXECUTE_IF_OFF);
        }
        // ---------- The above is to distinguish if the payload is present or not

        if (READBITS(optionMask, EMBER_ZCL_LEVEL_CONTROL_OPTIONS_EXECUTE_IF_OFF))
        {
            // Mask is present and set in the command payload, this indicates
            // use the over ride as temporary option
            return READBITS(optionOverride, EMBER_ZCL_LEVEL_CONTROL_OPTIONS_EXECUTE_IF_OFF);
        }
        // if we are here - use the option bits
        return (READBITS(options, EMBER_ZCL_LEVEL_CONTROL_OPTIONS_EXECUTE_IF_OFF));
    }

#endif // IGNORE_LEVEL_CONTROL_CLUSTER_OPTIONS
    // By default, we return true to continue supporting backwards compatibility.
    return true;
}

bool emberAfLevelControlClusterMoveToLevelCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                   const Commands::MoveToLevel::DecodableType & commandData)
{
    auto & level          = commandData.level;
    auto & transitionTime = commandData.transitionTime;
    auto & optionMask     = commandData.optionMask;
    auto & optionOverride = commandData.optionOverride;

    emberAfLevelControlClusterPrintln("%pMOVE_TO_LEVEL %x %2x %x %x", "RX level-control:", level, transitionTime, optionMask,
                                      optionOverride);
    EmberAfStatus status =
        moveToLevelHandler(commandPath.mEndpointId, Commands::MoveToLevel::Id, level, transitionTime, optionMask, optionOverride,
                           INVALID_STORED_LEVEL); // Don't revert to the stored level

    emberAfSendImmediateDefaultResponse(status);

    return true;
}

bool emberAfLevelControlClusterMoveToLevelWithOnOffCallback(app::CommandHandler * commandObj,
                                                            const app::ConcreteCommandPath & commandPath,
                                                            const Commands::MoveToLevelWithOnOff::DecodableType & commandData)
{
    auto & level          = commandData.level;
    auto & transitionTime = commandData.transitionTime;

    emberAfLevelControlClusterPrintln("%pMOVE_TO_LEVEL_WITH_ON_OFF %x %2x", "RX level-control:", level, transitionTime);
    EmberAfStatus status =
        moveToLevelHandler(commandPath.mEndpointId, Commands::MoveToLevelWithOnOff::Id, level, transitionTime, 0xFF, 0xFF,
                           INVALID_STORED_LEVEL); // Don't revert to the stored level

    emberAfSendImmediateDefaultResponse(status);

    return true;
}

bool emberAfLevelControlClusterMoveCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                            const Commands::Move::DecodableType & commandData)
{
    auto & moveMode       = commandData.moveMode;
    auto & rate           = commandData.rate;
    auto & optionMask     = commandData.optionMask;
    auto & optionOverride = commandData.optionOverride;

    emberAfLevelControlClusterPrintln("%pMOVE %x %x", "RX level-control:", moveMode, rate);
    moveHandler(Commands::Move::Id, moveMode, rate, optionMask, optionOverride);
    return true;
}

bool emberAfLevelControlClusterMoveWithOnOffCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                     const Commands::MoveWithOnOff::DecodableType & commandData)
{
    auto & moveMode = commandData.moveMode;
    auto & rate     = commandData.rate;

    emberAfLevelControlClusterPrintln("%pMOVE_WITH_ON_OFF %x %x", "RX level-control:", moveMode, rate);
    moveHandler(Commands::MoveWithOnOff::Id, moveMode, rate, 0xFF, 0xFF);
    return true;
}

bool emberAfLevelControlClusterStepCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                            const Commands::Step::DecodableType & commandData)
{
    auto & stepMode       = commandData.stepMode;
    auto & stepSize       = commandData.stepSize;
    auto & transitionTime = commandData.transitionTime;
    auto & optionMask     = commandData.optionMask;
    auto & optionOverride = commandData.optionOverride;

    emberAfLevelControlClusterPrintln("%pSTEP %x %x %2x", "RX level-control:", stepMode, stepSize, transitionTime);
    stepHandler(Commands::Step::Id, stepMode, stepSize, transitionTime, optionMask, optionOverride);
    return true;
}

bool emberAfLevelControlClusterStepWithOnOffCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                     const Commands::StepWithOnOff::DecodableType & commandData)
{
    auto & stepMode       = commandData.stepMode;
    auto & stepSize       = commandData.stepSize;
    auto & transitionTime = commandData.transitionTime;

    emberAfLevelControlClusterPrintln("%pSTEP_WITH_ON_OFF %x %x %2x", "RX level-control:", stepMode, stepSize, transitionTime);
    stepHandler(Commands::StepWithOnOff::Id, stepMode, stepSize, transitionTime, 0xFF, 0xFF);
    return true;
}

bool emberAfLevelControlClusterStopCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                            const Commands::Stop::DecodableType & commandData)
{
    auto & optionMask     = commandData.optionMask;
    auto & optionOverride = commandData.optionOverride;

    emberAfLevelControlClusterPrintln("%pSTOP", "RX level-control:");
    stopHandler(Commands::Stop::Id, optionMask, optionOverride);
    return true;
}

bool emberAfLevelControlClusterStopWithOnOffCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                     const Commands::StopWithOnOff::DecodableType & commandData)
{
    emberAfLevelControlClusterPrintln("%pSTOP_WITH_ON_OFF", "RX level-control:");
    stopHandler(Commands::StopWithOnOff::Id, 0xFF, 0xFF);
    return true;
}

static EmberAfStatus moveToLevelHandler(EndpointId endpoint, CommandId commandId, uint8_t level, uint16_t transitionTimeDs,
                                        uint8_t optionMask, uint8_t optionOverride, uint16_t storedLevel)
{
    EmberAfLevelControlState * state = getState(endpoint);
    EmberAfStatus status;
    uint8_t currentLevel;
    uint8_t actualStepSize;

    if (state == nullptr)
    {
        return EMBER_ZCL_STATUS_FAILURE;
    }

    if (level > EMBER_AF_PLUGIN_LEVEL_CONTROL_MAXIMUM_LEVEL)
    {
        return EMBER_ZCL_STATUS_INVALID_COMMAND;
    }

    if (!shouldExecuteIfOff(endpoint, commandId, optionMask, optionOverride))
    {
        return EMBER_ZCL_STATUS_SUCCESS;
    }

    // Cancel any currently active command before fiddling with the state.
    deactivate(endpoint);

    status = Attributes::CurrentLevel::Get(endpoint, &currentLevel);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfLevelControlClusterPrintln("ERR: reading current level %x", status);
        return status;
    }

    state->commandId = commandId;

    // Move To Level commands cause the device to move from its current level to
    // the specified level at the specified rate.
    if (state->maxLevel <= level)
    {
        state->moveToLevel = state->maxLevel;
    }
    else if (level <= state->minLevel)
    {
        state->moveToLevel = state->minLevel;
    }
    else
    {
        state->moveToLevel = level;
    }

    // If the level is decreasing, the On/Off attribute is left unchanged.  This
    // logic is to prevent a light from transitioning from off to bright to dim.
    // Instead, a light that is off will stay off until the target level is
    // reached.
    if (currentLevel <= state->moveToLevel)
    {
        if (commandId == Commands::MoveToLevelWithOnOff::Id)
        {
            setOnOffValue(endpoint, (state->moveToLevel != state->minLevel));
        }
        if (currentLevel == state->moveToLevel)
        {
            return EMBER_ZCL_STATUS_SUCCESS;
        }
        state->increasing = true;
        actualStepSize    = static_cast<uint8_t>(state->moveToLevel - currentLevel);
    }
    else
    {
        state->increasing = false;
        actualStepSize    = static_cast<uint8_t>(currentLevel - state->moveToLevel);
    }

    // If the Transition time field takes the value 0xFFFF, then the time taken
    // to move to the new level shall instead be determined by the On/Off
    // Transition Time attribute.  If On/Off Transition Time, which is an
    // optional attribute, is not present, the device shall move to its new level
    // as fast as it is able.
    if (transitionTimeDs == 0xFFFF)
    {
#ifndef IGNORE_LEVEL_CONTROL_CLUSTER_ON_OFF_TRANSITION_TIME
        if (emberAfContainsAttribute(endpoint, LevelControl::Id, Attributes::OnOffTransitionTime::Id))
        {
            status = Attributes::OnOffTransitionTime::Get(endpoint, &transitionTimeDs);
            if (status != EMBER_ZCL_STATUS_SUCCESS)
            {
                emberAfLevelControlClusterPrintln("ERR: reading on/off transition time %x", status);
                return status;
            }

            // Transition time comes in (or is stored, in the case of On/Off Transition
            // Time) as tenths of a second, but we work in milliseconds.
            state->transitionTimeMs = (transitionTimeDs * MILLISECOND_TICKS_PER_SECOND / 10);
        }
        else
        {
            state->transitionTimeMs = FASTEST_TRANSITION_TIME_MS;
        }
#else
        // If the Transition Time field is 0xFFFF and On/Off Transition Time,
        // which is an optional attribute, is not present, the device shall move to
        // its new level as fast as it is able.
        state->transitionTimeMs = FASTEST_TRANSITION_TIME_MS;
#endif // IGNORE_LEVEL_CONTROL_CLUSTER_ON_OFF_TRANSITION_TIME
    }
    else
    {
        // Transition time comes in (or is stored, in the case of On/Off Transition
        // Time) as tenths of a second, but we work in milliseconds.
        state->transitionTimeMs = (transitionTimeDs * MILLISECOND_TICKS_PER_SECOND / 10);
    }

    // The duration between events will be the transition time divided by the
    // distance we must move.
    state->eventDurationMs = state->transitionTimeMs / actualStepSize;
    state->elapsedTimeMs   = 0;

    // OnLevel is not used for Move commands.
    state->useOnLevel = false;

    state->storedLevel = storedLevel;

    // The setup was successful, so mark the new state as active and return.
    schedule(endpoint, state->eventDurationMs);
    status = EMBER_ZCL_STATUS_SUCCESS;

    if (commandId == Commands::MoveToLevelWithOnOff::Id)
    {
        uint32_t featureMap;
        if (Attributes::FeatureMap::Get(endpoint, &featureMap) == EMBER_ZCL_STATUS_SUCCESS &&
            READBITS(featureMap, EMBER_AF_LEVEL_CONTROL_FEATURE_LIGHTING))
        {
            OnOff::Attributes::GlobalSceneControl::Set(endpoint, true);
        }
    }

    return status;
}

static void moveHandler(CommandId commandId, uint8_t moveMode, uint8_t rate, uint8_t optionMask, uint8_t optionOverride)
{
    EndpointId endpoint              = emberAfCurrentEndpoint();
    EmberAfLevelControlState * state = getState(endpoint);
    EmberAfStatus status;
    uint8_t currentLevel;
    uint8_t difference;

    if (state == nullptr)
    {
        status = EMBER_ZCL_STATUS_FAILURE;
        goto send_default_response;
    }

    if (rate == 0 || !shouldExecuteIfOff(endpoint, commandId, optionMask, optionOverride))
    {
        status = EMBER_ZCL_STATUS_SUCCESS;
        goto send_default_response;
    }

    // Cancel any currently active command before fiddling with the state.
    deactivate(endpoint);

    status = Attributes::CurrentLevel::Get(endpoint, &currentLevel);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfLevelControlClusterPrintln("ERR: reading current level %x", status);
        goto send_default_response;
    }

    state->commandId = commandId;

    // Move commands cause the device to move from its current level to either
    // the maximum or minimum level at the specified rate.
    switch (moveMode)
    {
    case EMBER_ZCL_MOVE_MODE_UP:
        state->increasing  = true;
        state->moveToLevel = state->maxLevel;
        difference         = static_cast<uint8_t>(state->maxLevel - currentLevel);
        break;
    case EMBER_ZCL_MOVE_MODE_DOWN:
        state->increasing  = false;
        state->moveToLevel = state->minLevel;
        difference         = static_cast<uint8_t>(currentLevel - state->minLevel);
        break;
    default:
        status = EMBER_ZCL_STATUS_INVALID_FIELD;
        goto send_default_response;
    }

    // If the level is decreasing, the On/Off attribute is left unchanged.  This
    // logic is to prevent a light from transitioning from off to bright to dim.
    // Instead, a light that is off will stay off until the target level is
    // reached.
    if (currentLevel <= state->moveToLevel)
    {
        if (commandId == Commands::MoveWithOnOff::Id)
        {
            setOnOffValue(endpoint, (state->moveToLevel != state->minLevel));
        }
        if (currentLevel == state->moveToLevel)
        {
            status = EMBER_ZCL_STATUS_SUCCESS;
            goto send_default_response;
        }
    }

    // If the Rate field is 0xFF, the device should move at the default move rate, if available,
    // Otherwise, move as fast as possible
    if (rate == 0xFF)
    {
        app::DataModel::Nullable<uint8_t> defaultMoveRate;
        status = Attributes::DefaultMoveRate::Get(endpoint, defaultMoveRate);
        if (status != EMBER_ZCL_STATUS_SUCCESS || defaultMoveRate.IsNull())
        {
            emberAfLevelControlClusterPrintln("ERR: reading default move rate %x", status);
            state->eventDurationMs = FASTEST_TRANSITION_TIME_MS;
        }
        else
        {
            // nonsensical case, means "don't move", so we're done
            if (defaultMoveRate.Value() == 0)
            {
                status = EMBER_ZCL_STATUS_SUCCESS;
                goto send_default_response;
            }
            state->eventDurationMs = MILLISECOND_TICKS_PER_SECOND / defaultMoveRate.Value();
        }
    }
    else
    {
        state->eventDurationMs = MILLISECOND_TICKS_PER_SECOND / rate;
    }

    state->transitionTimeMs = difference * state->eventDurationMs;
    state->elapsedTimeMs    = 0;

    // OnLevel is not used for Move commands.
    state->useOnLevel = false;

    // storedLevel is not used for Move commands.
    state->storedLevel = INVALID_STORED_LEVEL;

    // The setup was successful, so mark the new state as active and return.
    schedule(endpoint, state->eventDurationMs);
    status = EMBER_ZCL_STATUS_SUCCESS;

send_default_response:
    emberAfSendImmediateDefaultResponse(status);
}

static void stepHandler(CommandId commandId, uint8_t stepMode, uint8_t stepSize, uint16_t transitionTimeDs, uint8_t optionMask,
                        uint8_t optionOverride)
{
    EndpointId endpoint              = emberAfCurrentEndpoint();
    EmberAfLevelControlState * state = getState(endpoint);
    EmberAfStatus status;
    uint8_t currentLevel;
    uint8_t actualStepSize = stepSize;

    if (state == nullptr)
    {
        status = EMBER_ZCL_STATUS_FAILURE;
        goto send_default_response;
    }

    if (!shouldExecuteIfOff(endpoint, commandId, optionMask, optionOverride))
    {
        status = EMBER_ZCL_STATUS_SUCCESS;
        goto send_default_response;
    }

    // Cancel any currently active command before fiddling with the state.
    deactivate(endpoint);

    status = Attributes::CurrentLevel::Get(endpoint, &currentLevel);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfLevelControlClusterPrintln("ERR: reading current level %x", status);
        goto send_default_response;
    }

    state->commandId = commandId;

    // Step commands cause the device to move from its current level to a new
    // level over the specified transition time.
    switch (stepMode)
    {
    case EMBER_ZCL_STEP_MODE_UP:
        state->increasing = true;
        if (state->maxLevel - currentLevel < stepSize)
        {
            state->moveToLevel = state->maxLevel;
            actualStepSize     = static_cast<uint8_t>(state->maxLevel - currentLevel);
        }
        else
        {
            state->moveToLevel = static_cast<uint8_t>(currentLevel + stepSize);
        }
        break;
    case EMBER_ZCL_STEP_MODE_DOWN:
        state->increasing = false;
        if (currentLevel - state->minLevel < stepSize)
        {
            state->moveToLevel = state->minLevel;
            actualStepSize     = static_cast<uint8_t>(currentLevel - state->minLevel);
        }
        else
        {
            state->moveToLevel = static_cast<uint8_t>(currentLevel - stepSize);
        }
        break;
    default:
        status = EMBER_ZCL_STATUS_INVALID_FIELD;
        goto send_default_response;
    }

    // If the level is decreasing, the On/Off attribute is left unchanged.  This
    // logic is to prevent a light from transitioning from off to bright to dim.
    // Instead, a light that is off will stay off until the target level is
    // reached.
    if (currentLevel <= state->moveToLevel)
    {
        if (commandId == Commands::StepWithOnOff::Id)
        {
            setOnOffValue(endpoint, (state->moveToLevel != state->minLevel));
        }
        if (currentLevel == state->moveToLevel)
        {
            status = EMBER_ZCL_STATUS_SUCCESS;
            goto send_default_response;
        }
    }

    // If the Transition Time field is 0xFFFF, the device should move as fast as
    // it is able.
    if (transitionTimeDs == 0xFFFF)
    {
        state->transitionTimeMs = FASTEST_TRANSITION_TIME_MS;
    }
    else
    {
        // Transition time comes in as tenths of a second, but we work in
        // milliseconds.
        state->transitionTimeMs = (transitionTimeDs * MILLISECOND_TICKS_PER_SECOND / 10);
        // If the new level was pegged at the minimum level, the transition time
        // shall be proportionally reduced.  This is done after the conversion to
        // milliseconds to reduce rounding errors in integer division.
        if (stepSize != actualStepSize)
        {
            state->transitionTimeMs = (state->transitionTimeMs * actualStepSize / stepSize);
        }
    }

    // The duration between events will be the transition time divided by the
    // distance we must move.
    state->eventDurationMs = state->transitionTimeMs / actualStepSize;
    state->elapsedTimeMs   = 0;

    // OnLevel is not used for Step commands.
    state->useOnLevel = false;

    // storedLevel is not used for Step commands
    state->storedLevel = INVALID_STORED_LEVEL;

    // The setup was successful, so mark the new state as active and return.
    schedule(endpoint, state->eventDurationMs);
    status = EMBER_ZCL_STATUS_SUCCESS;

send_default_response:
    emberAfSendImmediateDefaultResponse(status);
}

static void stopHandler(CommandId commandId, uint8_t optionMask, uint8_t optionOverride)
{
    EndpointId endpoint              = emberAfCurrentEndpoint();
    EmberAfLevelControlState * state = getState(endpoint);
    EmberAfStatus status;

    if (state == nullptr)
    {
        status = EMBER_ZCL_STATUS_FAILURE;
        goto send_default_response;
    }

    if (!shouldExecuteIfOff(endpoint, commandId, optionMask, optionOverride))
    {
        status = EMBER_ZCL_STATUS_SUCCESS;
        goto send_default_response;
    }

    // Cancel any currently active command.
    deactivate(endpoint);
    writeRemainingTime(endpoint, 0);
    status = EMBER_ZCL_STATUS_SUCCESS;

send_default_response:
    emberAfSendImmediateDefaultResponse(status);
}

// Follows 07-5123-04 (ZigBee Cluster Library doc), section 3.10.2.1.1.
// Quotes are from table 3.46.
void emberAfOnOffClusterLevelControlEffectCallback(EndpointId endpoint, bool newValue)
{
    app::DataModel::Nullable<uint8_t> resolvedLevel;
    uint8_t temporaryCurrentLevelCache;
    uint16_t currentOnOffTransitionTime;
    EmberAfStatus status;

    EmberAfLevelControlState * state = getState(endpoint);
    if (state == nullptr)
    {
        emberAfLevelControlClusterPrintln("ERR: Level control cluster not available on ep%d", endpoint);
        return;
    }

    uint8_t minimumLevelAllowedForTheDevice = state->minLevel;

    // "Temporarily store CurrentLevel."
    status = Attributes::CurrentLevel::Get(endpoint, &temporaryCurrentLevelCache);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfLevelControlClusterPrintln("ERR: reading current level %x", status);
        return;
    }

    // Read the OnLevel attribute.
#ifndef IGNORE_LEVEL_CONTROL_CLUSTER_ON_LEVEL_ATTRIBUTE
    if (emberAfContainsAttribute(endpoint, LevelControl::Id, Attributes::OnLevel::Id))
    {
        status = Attributes::OnLevel::Get(endpoint, resolvedLevel);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            emberAfLevelControlClusterPrintln("ERR: reading current level %x", status);
            return;
        }

        if (resolvedLevel.IsNull())
        {
            // OnLevel has undefined value; fall back to CurrentLevel.
            resolvedLevel.SetNonNull(temporaryCurrentLevelCache);
        }
    }
    else
    {
        resolvedLevel.SetNonNull(temporaryCurrentLevelCache);
    }
#else
    resolvedLevel.SetNonNull(temporaryCurrentLevelCache);
#endif // IGNORE_LEVEL_CONTROL_CLUSTER_ON_LEVEL_ATTRIBUTE

    // Read the OnOffTransitionTime attribute.
#ifndef IGNORE_LEVEL_CONTROL_CLUSTER_ON_OFF_TRANSITION_TIME
    if (emberAfContainsAttribute(endpoint, LevelControl::Id, Attributes::OnOffTransitionTime::Id))
    {
        status = Attributes::OnOffTransitionTime::Get(endpoint, &currentOnOffTransitionTime);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            emberAfLevelControlClusterPrintln("ERR: reading current level %x", status);
            return;
        }
    }
    else
    {
        currentOnOffTransitionTime = 0xFFFF;
    }
#else
    currentOnOffTransitionTime = 0xFFFF;
#endif // IGNORE_LEVEL_CONTROL_CLUSTER_ON_OFF_TRANSITION_TIME

    if (newValue)
    {
        // If newValue is OnOff::Commands::On::Id...
        // "Set CurrentLevel to minimum level allowed for the device."
        status = Attributes::CurrentLevel::Set(endpoint, minimumLevelAllowedForTheDevice);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            emberAfLevelControlClusterPrintln("ERR: reading current level %x", status);
            return;
        }

        // "Move CurrentLevel to OnLevel, or to the stored level if OnLevel is not
        // defined, over the time period OnOffTransitionTime."
        moveToLevelHandler(endpoint, Commands::MoveToLevel::Id, resolvedLevel.Value(), currentOnOffTransitionTime, 0xFF, 0xFF,
                           INVALID_STORED_LEVEL); // Don't revert to stored level
    }
    else
    {
        // ...else if newValue is OnOff::Commands::Off::Id...
        // "Move CurrentLevel to the minimum level allowed for the device over the
        // time period OnOffTransitionTime."
        moveToLevelHandler(endpoint, Commands::MoveToLevel::Id, minimumLevelAllowedForTheDevice, currentOnOffTransitionTime, 0xFF,
                           0xFF, temporaryCurrentLevelCache);

        // "If OnLevel is not defined, set the CurrentLevel to the stored level."
        // The emberAfLevelControlClusterServerTickCallback implementation handles
        // this.
    }
}

void emberAfLevelControlClusterServerInitCallback(EndpointId endpoint)
{
    EmberAfLevelControlState * state = getState(endpoint);

    if (state == nullptr)
    {
        emberAfLevelControlClusterPrintln("ERR: Level control cluster not available on ep%d", endpoint);
        return;
    }

    state->minLevel = EMBER_AF_PLUGIN_LEVEL_CONTROL_MINIMUM_LEVEL;
    state->maxLevel = EMBER_AF_PLUGIN_LEVEL_CONTROL_MAXIMUM_LEVEL;

    // If these read only attributes are enabled we use those values as our set minLevel and maxLevel
    // if get isn't possible, value stays at default
    Attributes::MinLevel::Get(endpoint, &state->minLevel);
    Attributes::MaxLevel::Get(endpoint, &state->maxLevel);

    uint32_t featureMap;
    if (Attributes::FeatureMap::Get(endpoint, &featureMap) == EMBER_ZCL_STATUS_SUCCESS &&
        READBITS(featureMap, EMBER_AF_LEVEL_CONTROL_FEATURE_LIGHTING))
    {
        if (state->minLevel < LEVEL_CONTROL_LIGHTING_MIN_LEVEL)
        {
            state->minLevel = LEVEL_CONTROL_LIGHTING_MIN_LEVEL;
        }

        if (state->maxLevel > LEVEL_CONTROL_LIGHTING_MAX_LEVEL)
        {
            state->maxLevel = LEVEL_CONTROL_LIGHTING_MAX_LEVEL;
        }
    }

    uint8_t currentLevel = 0;
    EmberAfStatus status = Attributes::CurrentLevel::Get(endpoint, &currentLevel);
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
#ifndef IGNORE_LEVEL_CONTROL_CLUSTER_START_UP_CURRENT_LEVEL
        // StartUp behavior relies StartUpCurrentLevel attributes being Non Volatile.
        if (areStartUpLevelControlServerAttributesNonVolatile(endpoint))
        {
            // 1.5.14. StartUpCurrentLevel Attribute
            // The StartUpCurrentLevel attribute SHALL define the desired startup level
            // for a device when it is supplied with power and this level SHALL be
            // reflected in the CurrentLevel attribute. The values of the StartUpCurrentLevel
            // attribute are listed below:
            // Table 4. Values of the StartUpCurrentLevel attribute
            // Value      Action on power up
            // 0x00       Set the CurrentLevel attribute to the minimum value permitted on the device.
            // 0x01-0xfe  Set the CurrentLevel attribute to this value.
            // NULL       Set the CurrentLevel attribute to its previous value.
            // 0xFF       Work Around ZAP Can't set default value to NULL
            // https://github.com/project-chip/zap/issues/354

            app::DataModel::Nullable<uint8_t> startUpCurrentLevel;
            status = Attributes::StartUpCurrentLevel::Get(endpoint, startUpCurrentLevel);
            if (status == EMBER_ZCL_STATUS_SUCCESS)
            {
                if (!startUpCurrentLevel.IsNull())
                {
                    if (startUpCurrentLevel.Value() == STARTUP_CURRENT_LEVEL_USE_DEVICE_MINIMUM)
                    {
                        currentLevel = state->minLevel;
                    }
                    else
                    {
                        // Otherwise set to specified value 0x01-0xFE.
                        // But, need to enforce currentLevel's min/max, right?
                        // Spec doesn't mention this.
                        if (startUpCurrentLevel.Value() < state->minLevel)
                        {
                            currentLevel = state->minLevel;
                        }
                        else if (startUpCurrentLevel.Value() > state->maxLevel)
                        {
                            currentLevel = state->maxLevel;
                        }
                        else
                        {
                            currentLevel = startUpCurrentLevel.Value();
                        }
                    }
                }
                // Otherwise Set the CurrentLevel attribute to its previous value which was already fetch above

                Attributes::CurrentLevel::Set(endpoint, currentLevel);
            }
        }
#endif // IGNORE_LEVEL_CONTROL_CLUSTER_START_UP_CURRENT_LEVEL
       // In any case, we make sure that the respects min/max
        if (currentLevel < state->minLevel)
        {
            Attributes::CurrentLevel::Set(endpoint, state->minLevel);
        }
        else if (currentLevel > state->maxLevel)
        {
            Attributes::CurrentLevel::Set(endpoint, state->maxLevel);
        }
    }

    emberAfPluginLevelControlClusterServerPostInitCallback(endpoint);
}

#ifndef IGNORE_LEVEL_CONTROL_CLUSTER_START_UP_CURRENT_LEVEL
static bool areStartUpLevelControlServerAttributesNonVolatile(EndpointId endpoint)
{
    if (emberAfIsNonVolatileAttribute(endpoint, LevelControl::Id, Attributes::CurrentLevel::Id))
    {
        return emberAfIsNonVolatileAttribute(endpoint, LevelControl::Id, Attributes::StartUpCurrentLevel::Id);
    }

    return false;
}
#endif // IGNORE_LEVEL_CONTROL_CLUSTER_START_UP_CURRENT_LEVEL

void emberAfPluginLevelControlClusterServerPostInitCallback(EndpointId endpoint) {}

void MatterLevelControlPluginServerInitCallback() {}
