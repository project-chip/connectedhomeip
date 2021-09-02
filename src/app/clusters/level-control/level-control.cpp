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
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/command-id.h>
#include <app/CommandHandler.h>
#include <app/util/af.h>

#include <app/reporting/reporting.h>

#ifdef EMBER_AF_PLUGIN_SCENES
#include <app/clusters/scenes/scenes.h>
#endif // EMBER_AF_PLUGIN_SCENES

#ifdef EMBER_AF_PLUGIN_ON_OFF
#include <app/clusters/on-off-server/on-off-server.h>
#endif // EMBER_AF_PLUGIN_ON_OFF

#ifdef EMBER_AF_PLUGIN_ZLL_LEVEL_CONTROL_SERVER
#include "app/framework/plugin/zll-level-control-server/zll-level-control-server.h"
#endif // EMBER_AF_PLUGIN_ZLL_LEVEL_CONTROL_SERVER

#include <assert.h>

using namespace chip;

#ifdef ZCL_USING_LEVEL_CONTROL_CLUSTER_START_UP_CURRENT_LEVEL_ATTRIBUTE
static bool areStartUpLevelControlServerAttributesTokenized(EndpointId endpoint);
#endif

#if (EMBER_AF_PLUGIN_LEVEL_CONTROL_RATE == 0)
#define FASTEST_TRANSITION_TIME_MS 0
#else
#define FASTEST_TRANSITION_TIME_MS (MILLISECOND_TICKS_PER_SECOND / EMBER_AF_PLUGIN_LEVEL_CONTROL_RATE)
#endif

#ifdef EMBER_AF_PLUGIN_ZLL_LEVEL_CONTROL_SERVER
#define MIN_LEVEL EMBER_AF_PLUGIN_ZLL_LEVEL_CONTROL_SERVER_MINIMUM_LEVEL
#define MAX_LEVEL EMBER_AF_PLUGIN_ZLL_LEVEL_CONTROL_SERVER_MAXIMUM_LEVEL
#else
#define MIN_LEVEL EMBER_AF_PLUGIN_LEVEL_CONTROL_MINIMUM_LEVEL
#define MAX_LEVEL EMBER_AF_PLUGIN_LEVEL_CONTROL_MAXIMUM_LEVEL
#endif

#define INVALID_STORED_LEVEL 0xFFFF

#define STARTUP_CURRENT_LEVEL_USE_DEVICE_MINIMUM 0x00
#define STARTUP_CURRENT_LEVEL_USE_PREVIOUS_LEVEL 0xFF

typedef struct
{
    CommandId commandId;
    uint8_t moveToLevel;
    bool increasing;
    bool useOnLevel;
    uint8_t onLevel;
    uint16_t storedLevel;
    uint32_t eventDurationMs;
    uint32_t transitionTimeMs;
    uint32_t elapsedTimeMs;
} EmberAfLevelControlState;

static EmberAfLevelControlState stateTable[EMBER_AF_LEVEL_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT];

static EmberAfLevelControlState * getState(EndpointId endpoint);

static void moveToLevelHandler(CommandId commandId, uint8_t level, uint16_t transitionTimeDs, uint8_t optionMask,
                               uint8_t optionOverride, uint16_t storedLevel);
static void moveHandler(CommandId commandId, uint8_t moveMode, uint8_t rate, uint8_t optionMask, uint8_t optionOverride);
static void stepHandler(CommandId commandId, uint8_t stepMode, uint8_t stepSize, uint16_t transitionTimeDs, uint8_t optionMask,
                        uint8_t optionOverride);
static void stopHandler(CommandId commandId, uint8_t optionMask, uint8_t optionOverride);

static void setOnOffValue(EndpointId endpoint, bool onOff);
static void writeRemainingTime(EndpointId endpoint, uint16_t remainingTimeMs);
static bool shouldExecuteIfOff(EndpointId endpoint, CommandId commandId, uint8_t optionMask, uint8_t optionOverride);

#if defined(ZCL_USING_LEVEL_CONTROL_CLUSTER_OPTIONS_ATTRIBUTE) && defined(EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_TEMP)
static void reallyUpdateCoupledColorTemp(EndpointId endpoint);
#define updateCoupledColorTemp(endpoint) reallyUpdateCoupledColorTemp(endpoint)
#else
#define updateCoupledColorTemp(endpoint)
#endif // LEVEL...OPTIONS_ATTRIBUTE && COLOR...SERVER_TEMP

static void schedule(EndpointId endpoint, uint32_t delayMs)
{
    emberAfScheduleServerTickExtended(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID, delayMs, EMBER_AF_LONG_POLL, EMBER_AF_OK_TO_SLEEP);
}

static void deactivate(EndpointId endpoint)
{
    emberAfDeactivateServerTick(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID);
}

static EmberAfLevelControlState * getState(EndpointId endpoint)
{
    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID);
    return (ep == 0xFFFF ? NULL : &stateTable[ep]);
}

#if defined(ZCL_USING_LEVEL_CONTROL_CLUSTER_OPTIONS_ATTRIBUTE) && defined(EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_TEMP)
static void reallyUpdateCoupledColorTemp(EndpointId endpoint)
{
    uint8_t options;
    EmberAfStatus status =
        emberAfReadServerAttribute(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_OPTIONS_ATTRIBUTE_ID, &options, sizeof(options));
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfLevelControlClusterPrintln("Unable to read Options attribute: 0x%X", status);
        return;
    }

    if (READBITS(options, EMBER_ZCL_LEVEL_CONTROL_OPTIONS_COUPLE_COLOR_TEMP_TO_LEVEL))
    {
        emberAfPluginLevelControlCoupledColorTempChangeCallback(endpoint);
    }
}
#endif // LEVEL...OPTIONS_ATTRIBUTE && COLOR...SERVER_TEMP

void emberAfLevelControlClusterServerTickCallback(EndpointId endpoint)
{
    EmberAfLevelControlState * state = getState(endpoint);
    EmberAfStatus status;
    uint8_t currentLevel;

    if (state == NULL)
    {
        return;
    }

    state->elapsedTimeMs += state->eventDurationMs;

#if !defined(ZCL_USING_LEVEL_CONTROL_CLUSTER_OPTIONS_ATTRIBUTE) && defined(EMBER_AF_PLUGIN_ZLL_LEVEL_CONTROL_SERVER)
    if (emberAfPluginZllLevelControlServerIgnoreMoveToLevelMoveStepStop(endpoint, state->commandId))
    {
        return;
    }
#endif

    // Read the attribute; print error message and return if it can't be read
    status = emberAfReadServerAttribute(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_CURRENT_LEVEL_ATTRIBUTE_ID,
                                        (uint8_t *) &currentLevel, sizeof(currentLevel));
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
        assert(currentLevel < MAX_LEVEL);
        assert(currentLevel < state->moveToLevel);
        currentLevel++;
    }
    else
    {
        assert(MIN_LEVEL < currentLevel);
        assert(state->moveToLevel < currentLevel);
        currentLevel--;
    }

    emberAfLevelControlClusterPrint(" to %d ", currentLevel);
    emberAfLevelControlClusterPrintln("(diff %c1)", state->increasing ? '+' : '-');

    status = emberAfWriteServerAttribute(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_CURRENT_LEVEL_ATTRIBUTE_ID,
                                         (uint8_t *) &currentLevel, ZCL_INT8U_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfLevelControlClusterPrintln("ERR: writing current level %x", status);
        writeRemainingTime(endpoint, 0);
        return;
    }

    updateCoupledColorTemp(endpoint);

#ifdef EMBER_AF_PLUGIN_SCENES
    // The level has changed, so the scene is no longer valid.
    if (emberAfContainsServer(endpoint, ZCL_SCENES_CLUSTER_ID))
    {
        emberAfScenesClusterMakeInvalidCallback(endpoint);
    }
#endif

    // Are we at the requested level?
    if (currentLevel == state->moveToLevel)
    {
        if (state->commandId == ZCL_MOVE_TO_LEVEL_WITH_ON_OFF_COMMAND_ID || state->commandId == ZCL_MOVE_WITH_ON_OFF_COMMAND_ID ||
            state->commandId == ZCL_STEP_WITH_ON_OFF_COMMAND_ID)
        {
            setOnOffValue(endpoint, (currentLevel != MIN_LEVEL));
            if (currentLevel == MIN_LEVEL && state->useOnLevel)
            {
                status = emberAfWriteServerAttribute(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_CURRENT_LEVEL_ATTRIBUTE_ID,
                                                     (uint8_t *) &state->onLevel, ZCL_INT8U_ATTRIBUTE_TYPE);
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
                status = emberAfWriteServerAttribute(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_CURRENT_LEVEL_ATTRIBUTE_ID,
                                                     (uint8_t *) &storedLevel8u, ZCL_INT8U_ATTRIBUTE_TYPE);
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
#ifdef ZCL_USING_LEVEL_CONTROL_CLUSTER_LEVEL_CONTROL_REMAINING_TIME_ATTRIBUTE
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
    uint16_t remainingTimeDs = (remainingTimeMs + 99) / 100;
    EmberStatus status =
        emberAfWriteServerAttribute(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_LEVEL_CONTROL_REMAINING_TIME_ATTRIBUTE_ID,
                                    (uint8_t *) &remainingTimeDs, ZCL_INT16U_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfLevelControlClusterPrintln("ERR: writing remaining time %x", status);
    }
#endif
}

static void setOnOffValue(EndpointId endpoint, bool onOff)
{
#ifdef EMBER_AF_PLUGIN_ON_OFF
    if (emberAfContainsServer(endpoint, ZCL_ON_OFF_CLUSTER_ID))
    {
        emberAfLevelControlClusterPrintln("Setting on/off to %p due to level change", onOff ? "ON" : "OFF");
        emberAfOnOffClusterSetValueCallback(endpoint, (onOff ? ZCL_ON_COMMAND_ID : ZCL_OFF_COMMAND_ID), true);
    }
#endif // EMBER_AF_PLUGIN_ON_OFF
}

static bool shouldExecuteIfOff(EndpointId endpoint, CommandId commandId, uint8_t optionMask, uint8_t optionOverride)
{
#ifdef ZCL_USING_LEVEL_CONTROL_CLUSTER_OPTIONS_ATTRIBUTE
    // From 3.10.2.2.8.1 of ZCL7 document 14-0127-20j-zcl-ch-3-general.docx:
    //   "Command execution SHALL NOT continue beyond the Options processing if
    //    all of these criteria are true:
    //      - The command is one of the ‘without On/Off’ commands: Move, Move to
    //        Level, Stop, or Step.
    //      - The On/Off cluster exists on the same endpoint as this cluster.
    //      - The OnOff attribute of the On/Off cluster, on this endpoint, is 0x00
    //        (FALSE).
    //      - The value of the ExecuteIfOff bit is 0."
    if (commandId > ZCL_STOP_COMMAND_ID)
    {
        return true;
    }

    if (!emberAfContainsServer(endpoint, ZCL_ON_OFF_CLUSTER_ID))
    {
        return true;
    }

    uint8_t options;
    EmberAfStatus status =
        emberAfReadServerAttribute(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_OPTIONS_ATTRIBUTE_ID, &options, sizeof(options));
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfLevelControlClusterPrintln("Unable to read Options attribute: 0x%X", status);
        // If we can't read the attribute, then we should just assume that it has its
        // default value.
        options = 0x00;
    }

    bool on;
    status = emberAfReadServerAttribute(endpoint, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, (uint8_t *) &on, sizeof(on));
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

    // ---------- The following order is important in decission making -------
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

#else
    // By default, we return true to continue supporting backwards compatibility.
    return true;
#endif
}

bool emberAfLevelControlClusterMoveToLevelCallback(EndpointId endpoint, app::CommandHandler * commandObj, uint8_t level,
                                                   uint16_t transitionTime, uint8_t optionMask, uint8_t optionOverride)
{
    emberAfLevelControlClusterPrintln("%pMOVE_TO_LEVEL %x %2x %x %x", "RX level-control:", level, transitionTime, optionMask,
                                      optionOverride);
    moveToLevelHandler(ZCL_MOVE_TO_LEVEL_COMMAND_ID, level, transitionTime, optionMask, optionOverride,
                       INVALID_STORED_LEVEL); // Don't revert to the stored level
    return true;
}

bool emberAfLevelControlClusterMoveToLevelWithOnOffCallback(EndpointId endpoint, app::CommandHandler * commandObj, uint8_t level,
                                                            uint16_t transitionTime)
{
    emberAfLevelControlClusterPrintln("%pMOVE_TO_LEVEL_WITH_ON_OFF %x %2x", "RX level-control:", level, transitionTime);
    moveToLevelHandler(ZCL_MOVE_TO_LEVEL_WITH_ON_OFF_COMMAND_ID, level, transitionTime, 0xFF, 0xFF,
                       INVALID_STORED_LEVEL); // Don't revert to the stored level
    return true;
}

bool emberAfLevelControlClusterMoveCallback(EndpointId endpoint, app::CommandHandler * commandObj, uint8_t moveMode, uint8_t rate,
                                            uint8_t optionMask, uint8_t optionOverride)
{
    emberAfLevelControlClusterPrintln("%pMOVE %x %x", "RX level-control:", moveMode, rate);
    moveHandler(ZCL_MOVE_COMMAND_ID, moveMode, rate, optionMask, optionOverride);
    return true;
}

bool emberAfLevelControlClusterMoveWithOnOffCallback(EndpointId endpoint, app::CommandHandler * commandObj, uint8_t moveMode,
                                                     uint8_t rate)
{
    emberAfLevelControlClusterPrintln("%pMOVE_WITH_ON_OFF %x %x", "RX level-control:", moveMode, rate);
    moveHandler(ZCL_MOVE_WITH_ON_OFF_COMMAND_ID, moveMode, rate, 0xFF, 0xFF);
    return true;
}

bool emberAfLevelControlClusterStepCallback(EndpointId endpoint, app::CommandHandler * commandObj, uint8_t stepMode,
                                            uint8_t stepSize, uint16_t transitionTime, uint8_t optionMask, uint8_t optionOverride)
{
    emberAfLevelControlClusterPrintln("%pSTEP %x %x %2x", "RX level-control:", stepMode, stepSize, transitionTime);
    stepHandler(ZCL_STEP_COMMAND_ID, stepMode, stepSize, transitionTime, optionMask, optionOverride);
    return true;
}

bool emberAfLevelControlClusterStepWithOnOffCallback(EndpointId endpoint, app::CommandHandler * commandObj, uint8_t stepMode,
                                                     uint8_t stepSize, uint16_t transitionTime)
{
    emberAfLevelControlClusterPrintln("%pSTEP_WITH_ON_OFF %x %x %2x", "RX level-control:", stepMode, stepSize, transitionTime);
    stepHandler(ZCL_STEP_WITH_ON_OFF_COMMAND_ID, stepMode, stepSize, transitionTime, 0xFF, 0xFF);
    return true;
}

bool emberAfLevelControlClusterStopCallback(EndpointId endpoint, app::CommandHandler * commandObj, uint8_t optionMask,
                                            uint8_t optionOverride)
{
    emberAfLevelControlClusterPrintln("%pSTOP", "RX level-control:");
    stopHandler(ZCL_STOP_COMMAND_ID, optionMask, optionOverride);
    return true;
}

bool emberAfLevelControlClusterStopWithOnOffCallback(EndpointId endpoint, app::CommandHandler * commandObj)
{
    emberAfLevelControlClusterPrintln("%pSTOP_WITH_ON_OFF", "RX level-control:");
    stopHandler(ZCL_STOP_WITH_ON_OFF_COMMAND_ID, 0xFF, 0xFF);
    return true;
}

static void moveToLevelHandler(CommandId commandId, uint8_t level, uint16_t transitionTimeDs, uint8_t optionMask,
                               uint8_t optionOverride, uint16_t storedLevel)
{
    EndpointId endpoint              = emberAfCurrentEndpoint();
    EmberAfLevelControlState * state = getState(endpoint);
    EmberAfStatus status;
    uint8_t currentLevel;
    uint8_t actualStepSize;

    if (state == NULL)
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

    status = emberAfReadServerAttribute(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_CURRENT_LEVEL_ATTRIBUTE_ID,
                                        (uint8_t *) &currentLevel, sizeof(currentLevel));
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfLevelControlClusterPrintln("ERR: reading current level %x", status);
        goto send_default_response;
    }

    state->commandId = commandId;

    // Move To Level commands cause the device to move from its current level to
    // the specified level at the specified rate.
    if (MAX_LEVEL <= level)
    {
        state->moveToLevel = MAX_LEVEL;
    }
    else if (level <= MIN_LEVEL)
    {
        state->moveToLevel = MIN_LEVEL;
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
        if (commandId == ZCL_MOVE_TO_LEVEL_WITH_ON_OFF_COMMAND_ID)
        {
            setOnOffValue(endpoint, (state->moveToLevel != MIN_LEVEL));
        }
        if (currentLevel == state->moveToLevel)
        {
            status = EMBER_ZCL_STATUS_SUCCESS;
            goto send_default_response;
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
#ifdef ZCL_USING_LEVEL_CONTROL_CLUSTER_ON_OFF_TRANSITION_TIME_ATTRIBUTE
        status = emberAfReadServerAttribute(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_ON_OFF_TRANSITION_TIME_ATTRIBUTE_ID,
                                            (uint8_t *) &transitionTimeDs, sizeof(transitionTimeDs));
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            emberAfLevelControlClusterPrintln("ERR: reading on/off transition time %x", status);
            goto send_default_response;
        }

        // Transition time comes in (or is stored, in the case of On/Off Transition
        // Time) as tenths of a second, but we work in milliseconds.
        state->transitionTimeMs = (transitionTimeDs * MILLISECOND_TICKS_PER_SECOND / 10);
#else  // ZCL_USING_LEVEL_CONTROL_CLUSTER_ON_OFF_TRANSITION_TIME_ATTRIBUTE
       // If the Transition Time field is 0xFFFF and On/Off Transition Time,
       // which is an optional attribute, is not present, the device shall move to
       // its new level as fast as it is able.
        state->transitionTimeMs = FASTEST_TRANSITION_TIME_MS;
#endif // ZCL_USING_LEVEL_CONTROL_CLUSTER_ON_OFF_TRANSITION_TIME_ATTRIBUTE
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

#ifdef EMBER_AF_PLUGIN_ZLL_LEVEL_CONTROL_SERVER
    if (commandId == ZCL_MOVE_TO_LEVEL_WITH_ON_OFF_COMMAND_ID)
    {
        emberAfPluginZllLevelControlServerMoveToLevelWithOnOffZllExtensions(emberAfCurrentCommand());
    }
#endif

send_default_response:
    if (emberAfCurrentCommand()->apsFrame->clusterId == ZCL_LEVEL_CONTROL_CLUSTER_ID)
    {
        emberAfSendImmediateDefaultResponse(status);
    }
}

static void moveHandler(CommandId commandId, uint8_t moveMode, uint8_t rate, uint8_t optionMask, uint8_t optionOverride)
{
    EndpointId endpoint              = emberAfCurrentEndpoint();
    EmberAfLevelControlState * state = getState(endpoint);
    EmberAfStatus status;
    uint8_t currentLevel;
    uint8_t difference;

    if (state == NULL)
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

    status = emberAfReadServerAttribute(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_CURRENT_LEVEL_ATTRIBUTE_ID,
                                        (uint8_t *) &currentLevel, sizeof(currentLevel));
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
        state->moveToLevel = MAX_LEVEL;
        difference         = static_cast<uint8_t>(MAX_LEVEL - currentLevel);
        break;
    case EMBER_ZCL_MOVE_MODE_DOWN:
        state->increasing  = false;
        state->moveToLevel = MIN_LEVEL;
        difference         = currentLevel - MIN_LEVEL;
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
        if (commandId == ZCL_MOVE_WITH_ON_OFF_COMMAND_ID)
        {
            setOnOffValue(endpoint, (state->moveToLevel != MIN_LEVEL));
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
        uint16_t defaultMoveRate;
        status = emberAfReadServerAttribute(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_DEFAULT_MOVE_RATE_ATTRIBUTE_ID,
                                            (uint8_t *) &defaultMoveRate, sizeof(defaultMoveRate));
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            emberAfLevelControlClusterPrintln("ERR: reading default move rate %x", status);
            state->eventDurationMs = FASTEST_TRANSITION_TIME_MS;
        }
        else
        {
            // nonsensical case, means "don't move", so we're done
            if (defaultMoveRate == 0)
            {
                status = EMBER_ZCL_STATUS_SUCCESS;
                goto send_default_response;
            }
            state->eventDurationMs = MILLISECOND_TICKS_PER_SECOND / defaultMoveRate;
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

    if (state == NULL)
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

    status = emberAfReadServerAttribute(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_CURRENT_LEVEL_ATTRIBUTE_ID,
                                        (uint8_t *) &currentLevel, sizeof(currentLevel));
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
        if (MAX_LEVEL - currentLevel < stepSize)
        {
            state->moveToLevel = MAX_LEVEL;
            actualStepSize     = static_cast<uint8_t>(MAX_LEVEL - currentLevel);
        }
        else
        {
            state->moveToLevel = static_cast<uint8_t>(currentLevel + stepSize);
        }
        break;
    case EMBER_ZCL_STEP_MODE_DOWN:
        state->increasing = false;
        if (currentLevel - MIN_LEVEL < stepSize)
        {
            state->moveToLevel = MIN_LEVEL;
            actualStepSize     = (currentLevel - MIN_LEVEL);
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
        if (commandId == ZCL_STEP_WITH_ON_OFF_COMMAND_ID)
        {
            setOnOffValue(endpoint, (state->moveToLevel != MIN_LEVEL));
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

    if (state == NULL)
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
    uint8_t temporaryCurrentLevelCache;
    uint16_t currentOnOffTransitionTime;
    uint8_t resolvedLevel;
    uint8_t minimumLevelAllowedForTheDevice = MIN_LEVEL;
    EmberAfStatus status;

    // "Temporarily store CurrentLevel."
    status = emberAfReadServerAttribute(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_CURRENT_LEVEL_ATTRIBUTE_ID,
                                        (uint8_t *) &temporaryCurrentLevelCache, sizeof(temporaryCurrentLevelCache));
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfLevelControlClusterPrintln("ERR: reading current level %x", status);
        return;
    }

    // Read the OnLevel attribute.
#ifdef ZCL_USING_LEVEL_CONTROL_CLUSTER_ON_LEVEL_ATTRIBUTE
    status = emberAfReadServerAttribute(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_ON_LEVEL_ATTRIBUTE_ID,
                                        (uint8_t *) &resolvedLevel, // OnLevel value
                                        sizeof(resolvedLevel));
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfLevelControlClusterPrintln("ERR: reading current level %x", status);
        return;
    }

    if (resolvedLevel == 0xFF)
    {
        // OnLevel has undefined value; fall back to CurrentLevel.
        resolvedLevel = temporaryCurrentLevelCache;
    }
#else
    resolvedLevel              = temporaryCurrentLevelCache;
#endif

    // Read the OnOffTransitionTime attribute.
#ifdef ZCL_USING_LEVEL_CONTROL_CLUSTER_ON_OFF_TRANSITION_TIME_ATTRIBUTE
    status = emberAfReadServerAttribute(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_ON_OFF_TRANSITION_TIME_ATTRIBUTE_ID,
                                        (uint8_t *) &currentOnOffTransitionTime, sizeof(currentOnOffTransitionTime));
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfLevelControlClusterPrintln("ERR: reading current level %x", status);
        return;
    }
#else
    currentOnOffTransitionTime = 0xFFFF;
#endif

    if (newValue)
    {
        // If newValue is ZCL_ON_COMMAND_ID...
        // "Set CurrentLevel to minimum level allowed for the device."
        status = emberAfWriteServerAttribute(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_CURRENT_LEVEL_ATTRIBUTE_ID,
                                             (uint8_t *) &minimumLevelAllowedForTheDevice, ZCL_INT8U_ATTRIBUTE_TYPE);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            emberAfLevelControlClusterPrintln("ERR: reading current level %x", status);
            return;
        }

        // "Move CurrentLevel to OnLevel, or to the stored level if OnLevel is not
        // defined, over the time period OnOffTransitionTime."
        moveToLevelHandler(ZCL_MOVE_TO_LEVEL_COMMAND_ID, resolvedLevel, currentOnOffTransitionTime, 0xFF, 0xFF,
                           INVALID_STORED_LEVEL); // Don't revert to stored level
    }
    else
    {
        // ...else if newValue is ZCL_OFF_COMMAND_ID...
        // "Move CurrentLevel to the minimum level allowed for the device over the
        // time period OnOffTransitionTime."
        moveToLevelHandler(ZCL_MOVE_TO_LEVEL_COMMAND_ID, minimumLevelAllowedForTheDevice, currentOnOffTransitionTime, 0xFF, 0xFF,
                           temporaryCurrentLevelCache);

        // "If OnLevel is not defined, set the CurrentLevel to the stored level."
        // The emberAfLevelControlClusterServerTickCallback implementation handles
        // this.
    }
}

void emberAfLevelControlClusterServerInitCallback(EndpointId endpoint)
{
#ifdef ZCL_USING_LEVEL_CONTROL_CLUSTER_START_UP_CURRENT_LEVEL_ATTRIBUTE
    // StartUp behavior relies StartUpCurrentLevel attributes being tokenized.
    if (areStartUpLevelControlServerAttributesTokenized(endpoint))
    {
        // Read the StartUpOnOff attribute and set the OnOff attribute as per
        // following from zcl 7 14-0127-20i-zcl-ch-3-general.doc.
        // 3.10.2.2.14	StartUpCurrentLevel Attribute
        // The StartUpCurrentLevel attribute SHALL define the desired startup level
        // for a device when it is supplied with power and this level SHALL be
        // reflected in the CurrentLevel attribute. The values of the StartUpCurrentLevel
        // attribute are listed below:
        // Table 3 58. Values of the StartUpCurrentLevel Attribute
        // Value      Action on power up
        // 0x00       Set the CurrentLevel attribute to the minimum value permitted on the device.
        // 0x01-0xfe  Set the CurrentLevel attribute to this value.
        // 0xff       Set the CurrentLevel attribute to its previous value.

        // Initialize startUpCurrentLevel to assume previous value for currentLevel.
        uint8_t startUpCurrentLevel = STARTUP_CURRENT_LEVEL_USE_PREVIOUS_LEVEL;
        EmberAfStatus status =
            emberAfReadAttribute(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_START_UP_CURRENT_LEVEL_ATTRIBUTE_ID,
                                 CLUSTER_MASK_SERVER, (uint8_t *) &startUpCurrentLevel, sizeof(startUpCurrentLevel), NULL);
        if (status == EMBER_ZCL_STATUS_SUCCESS)
        {
            uint8_t currentLevel = 0;
            status               = emberAfReadAttribute(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_CURRENT_LEVEL_ATTRIBUTE_ID,
                                          CLUSTER_MASK_SERVER, (uint8_t *) &currentLevel, sizeof(currentLevel), NULL);
            if (status == EMBER_ZCL_STATUS_SUCCESS)
            {
                switch (startUpCurrentLevel)
                {
                case STARTUP_CURRENT_LEVEL_USE_DEVICE_MINIMUM:
                    currentLevel = MIN_LEVEL;
                    break;
                case STARTUP_CURRENT_LEVEL_USE_PREVIOUS_LEVEL:
                    // Just fetched it.
                    break;
                default:
                    // Otherwise set to specified value 0x01-0xFE.
                    // But, need to enforce currentLevel's min/max, right?
                    // Spec doesn't mention this.
                    if (startUpCurrentLevel < MIN_LEVEL)
                    {
                        currentLevel = MIN_LEVEL;
                    }
                    else if (startUpCurrentLevel > MAX_LEVEL)
                    {
                        currentLevel = MAX_LEVEL;
                    }
                    else
                    {
                        currentLevel = startUpCurrentLevel;
                    }
                    break;
                }
                status = emberAfWriteAttribute(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_CURRENT_LEVEL_ATTRIBUTE_ID,
                                               CLUSTER_MASK_SERVER, (uint8_t *) &currentLevel, ZCL_INT8U_ATTRIBUTE_TYPE);
            }
        }
    }
#endif
    emberAfPluginLevelControlClusterServerPostInitCallback(endpoint);
}

#ifdef ZCL_USING_LEVEL_CONTROL_CLUSTER_START_UP_CURRENT_LEVEL_ATTRIBUTE
static bool areStartUpLevelControlServerAttributesTokenized(EndpointId endpoint)
{
    EmberAfAttributeMetadata * metadata;

    metadata = emberAfLocateAttributeMetadata(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_CURRENT_LEVEL_ATTRIBUTE_ID,
                                              CLUSTER_MASK_SERVER, EMBER_AF_NULL_MANUFACTURER_CODE);
    if (!emberAfAttributeIsTokenized(metadata))
    {
        return false;
    }

    metadata = emberAfLocateAttributeMetadata(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_START_UP_CURRENT_LEVEL_ATTRIBUTE_ID,
                                              CLUSTER_MASK_SERVER, EMBER_AF_NULL_MANUFACTURER_CODE);
    if (!emberAfAttributeIsTokenized(metadata))
    {
        return false;
    }

    return true;
}
#endif

void emberAfPluginLevelControlClusterServerPostInitCallback(EndpointId endpoint) {}
